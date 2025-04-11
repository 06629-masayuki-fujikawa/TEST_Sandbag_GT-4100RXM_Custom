/*[]----------------------------------------------------------------------[]
 *| System      : FT4800 他
 *| Module      : NT-NET自動送信制御
 *[]----------------------------------------------------------------------[]
 *| Author      : MATSUSHITA
 *| Date        : 2007. 2. 8
 *| Update      :
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"mem_def.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ntnet.h"
#include	"ntnetauto.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"rauconstant.h"
#include	"raudef.h"
#include	"ntnet_def.h"
#include	"mnt_def.h"

/*--------------------------------------------------------------------------
	defines
--------------------------------------------------------------------------*/
/* event bits to control this module */
#define	_TREV_SUS		0x0020			/* suspend */
#define	_TREV_RST		0x0010			/* restart */
#define	_TREV_NG		0x0002			/* request is not accepted */
#define	_TREV_OK		0x0001			/* request is accepted */

/* status bits to control this module:
	upper 4 bits are used for module control &
	middle 4 bits are reserved for time control &
	lower 8 bits are used for send transaction control
 */
#define	_TRSTS_DMNT		0x8000			/* automation module is domant */
#define	_TRSTS_PWON		0x4000			/* after power-on (waiting for clock setup) */
#define	_TRSTS_STP		0x0080			/* stop send-request control */
#define	_TRSTS_PEND		0x0010			/* pending transmission of send-request */
#define	_TRSTS_BSY		0x0008			/* transmitting send-request now */

/*	parameter read macros :
		adr=address in the section,
		no =data position left to right 1,2,3,4,5 or 6 (same as specication),
		cnt=number of positions	*/
#define	_REF_NTN(adr,no,cnt)	prm_get(0,S_NTN,(short)(adr),(char)(cnt),(char)(7-(no)-((cnt)-1)))
#define	_REF_CNT(adr,no,cnt)	prm_get(0,S_CEN,(short)(adr),(char)(cnt),(char)(7-(no)-((cnt)-1)))

#define	_INVALID_VALUE	9999
#define	_DISCARD(s)		s.year = _INVALID_VALUE
#define	_CHKDATE(s)		chkdate((short)s.year, (short)s.mont, (short)s.date)

/*--------------------------------------------------------------------------
	local area
--------------------------------------------------------------------------*/
static	uchar	prev_date;
static	uchar	prev_hour;
static	uchar	prev_min;
static	ushort	wup_flag;

/*--------------------------------------------------------------------------
	prototype for local functions
--------------------------------------------------------------------------*/
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
//static	int		send_parked_cars1(struct clk_rec *now);
//static	int		send_parked_cars2(struct clk_rec *now);
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し:ワーニング対策)
static	int		search_match_time(struct clk_rec *clk);
static	int		search_time_range(struct clk_rec *clk, int interval, int around,
								uchar *phour, uchar *pmin, ushort *pint);
static	int		get_next_pattern(int cur, int interval,
								uchar *phour, uchar *pmin, ushort *pint);

static	void	next_day(struct clk_rec *clk);
static	int		is_expired(struct clk_rec *now, struct clk_rec *app);
static	int		at_batch_time_teiji(struct clk_rec *now);
static	int		at_batch_time_teikankaku( struct clk_rec *now);
static	int		search_match_time_at_batch(struct clk_rec *clk);
static	int		search_time_range_at_batch(struct clk_rec *clk, int interval, int around,
								uchar *phour, uchar *pmin, ushort *pint);
static	int		get_next_pattern_at_batch(int cur, int interval,
								uchar *phour, uchar *pmin, ushort *pint);

extern 	char	z_NtRemoteAutoManual;

/*--------------------------------------------------------------------------
	function
--------------------------------------------------------------------------*/

/*[]----------------------------------------------------------------------[]
 *|	name	: initialize NT-NET automatic transmission control module
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET自動送信制御初期化
 *| param	: clr - 1=停電保証データもクリア
 *|			        0=停電保証データはクリアしない
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoInit(uchar clr)
{
	prev_date = 99;	/* invalid value */
	prev_hour = 99;	/* invalid value */
	prev_min = 99;		/* invalid value */
	wup_flag = 0;
	if (clr) {
		memset(&ntTranCtrl, 0, sizeof(ntTranCtrl));
		_DISCARD(ntTranTime.last0);
		_DISCARD(ntTranTime.last1);
		ntTranCtrl.status1 = _TRSTS_DMNT;
	}
	else {
		if (ntTranCtrl.event1 & _TREV_OK) {
			ntTranCtrl.status2 = 0;
		}
		ntTranCtrl.event1 = 0;
		if (_is_ntnet_remote()) {
			ntTranCtrl.status1 = 0;
			if (_CHKDATE(ntTranTime.last0) != 0)
				_DISCARD(ntTranTime.last0);
			if (_CHKDATE(ntTranTime.last1) != 0)
				_DISCARD(ntTranTime.last1);
			wup_flag = 1;
		}
		else {
			ntTranCtrl.status1 = _TRSTS_DMNT;
			_DISCARD(ntTranTime.last0);
			_DISCARD(ntTranTime.last1);
		}
	}
	ntTranCtrl.status1 |= _TRSTS_PWON;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: post time adjust
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET時刻更新
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoTimeAdjust(void)
{
	/* PWONのみ例外的に直接操作する */
	if (ntTranCtrl.status1 & _TRSTS_PWON)
		ntTranCtrl.status1 ^= _TRSTS_PWON;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: request to suspend or restart
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET自動制御の停止／再開
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoStartStop(ushort cond)
{
	if (cond) {
		// 定時送信の再開
		ntTranCtrl.event1 &= (~_TREV_SUS);
		ntTranCtrl.event1 |= _TREV_RST;
	}
	else {
		// 定時送信の停止
		ntTranCtrl.event1 &= (~_TREV_RST);
		ntTranCtrl.event1 |= _TREV_SUS;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: post result of send-request message to IBK
 *[]----------------------------------------------------------------------[]
 *| summary	: 要求電文の送信結果通知
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoPostResult(uchar kind, uchar OKNG)
{
	if (kind == 61) {
	/* after send-request message */
		ntTranCtrl.event1 |= (OKNG == 0)? _TREV_OK : _TREV_NG;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: post some tables became near-full
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETテーブルデータニアフル検出
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoPostNearFull(ulong data)
{
	// @todo 遠隔監視データのバッファニアフル時の送信設定が追加された場合、処理を追加すること
	static	const	char	chkset[][2] = {
//	bit0        1        2        3        4        5        6        7        8        9        10       11
//	入庫      出庫     精算     T合計     ｴﾗｰ     ｱﾗｰﾑ      ﾓﾆﾀ     操作      ｺｲﾝ     紙幣    駐車台数 釣銭管理
	{66, 1}, { 0, 0}, {66, 2}, {66, 3}, {66, 4}, {66, 5}, {66, 6}, {67, 1}, {67, 2}, {67, 3}, {67, 4}, {67, 5}};
	int		i;
	ulong	mask;
	ushort	set;
	ulong	reqAll;

	if (ntTranCtrl.status1 & _TRSTS_DMNT)
		return;

	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
		reqAll = NTNET_BUFCTRL_REQ_ALL;
	}
	else {
		reqAll = NTNET_BUFCTRL_REQ_ALL_PHASE1;
	}
	mask = 1L;
	data &= reqAll;
	for (i = 0; i < (sizeof(chkset)/sizeof(chkset[0])); i++) {
		if(i == 11) {
			mask <<= 2;
		}
		if (data & mask) {
			set = (ushort)_REF_NTN(chkset[i][0], chkset[i][1], 1);
			if (set == 2) {
				data = reqAll;
				break;				/* send call at this event */
			}
			else if (set == 0)
				data ^= mask;		/* bit off to dicard this event */
		}
		mask <<= 1;
	}
	ntTranCtrl.event2 |= data;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: request to send transaction
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET自動送信要求
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoReqToSend(ulong data)
{
	if (ntTranCtrl.status1 & _TRSTS_DMNT)
		return;
	if (_REF_CNT(51, 5, 1) != 0)
		return;
	ntTranCtrl.event2 |= data;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: control automatic transmission of NT-NET batch data
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET自動送信制御
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoSendCtrl(void)
{
	NTNET_AUTO_TRANCTRL		*ctrl = &ntTranCtrl;
	int		at_time;

	if ((ctrl->status1 & (_TRSTS_DMNT|_TRSTS_PWON)) != 0) {
		return;
	}

	/*--- time check ---*/
	if ( (CLK_REC.date != prev_date) ||
	     (CLK_REC.hour != prev_hour) ||
	     (CLK_REC.minu != prev_min) ) {
	/* minute updated */
		struct	clk_rec	now = CLK_REC;				/* get time */
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		/* send number of parked cars transaction */
//		switch(_REF_NTN(70, 6, 1)) {
//		case	1:
//			at_time = send_parked_cars1(&now);			/* appointed time */
//			break;
//		case	2:
//			at_time = send_parked_cars2(&now);			/* interval */
//			break;
//		default:
//			at_time = 0;
//			break;
//		}
//		if (at_time) {
//		/* now, send it */
//			ntTranTime.last1 = now;
//			Make_Log_ParkCarNum();						// 駐車台数データを生成
//			memcpy(&ParkCar_data_Bk, &ParkCar_data, sizeof(ParkCar_log));
//			Log_regist( LOG_PARKING );					// ログに登録(NT-NETタスク側で取り出す)
//		}
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	/* batch send */
		// バッチ送信時の動作
		switch(_REF_NTN(65, 2, 1)) {
		case	1:
			at_time = at_batch_time_teiji(&now);			/* appointed time */
			break;
		case	2:
			at_time = at_batch_time_teikankaku(&now);			/* interval */
			break;
		default:
			at_time = 0;
			break;
		}
		if (at_time) {
		/* now, request to send */
			ntTranTime.last0 = now;
			if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
				ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL);
			}
			else {
				ntautoReqToSend(NTNET_BUFCTRL_REQ_ALL_PHASE1);
			}
		}

		wup_flag = 0;
		ctrl->status1 &= (~_TRSTS_PEND);			/* clear pending to send */
		prev_min = CLK_REC.minu;
		prev_date = CLK_REC.date;
		prev_hour = CLK_REC.hour;
	}

	/*--- send transactions control ---*/
	if (ctrl->event1) {
	/* update control condition */
		if (ctrl->event1 & _TREV_SUS)
			ctrl->status1 |= _TRSTS_STP;				/* stop */
		else if (ctrl->event1 & _TREV_RST)
			ctrl->status1 &= (~_TRSTS_STP);			/* restart */

		if (ctrl->event1 & (_TREV_NG|_TREV_OK)) {
			if (ctrl->event1 & _TREV_NG)
				ctrl->status1 |= _TRSTS_PEND;		/* pending send until update minute */
			else
				ctrl->status2 = 0;					/* clear requesting data */
			ctrl->status1 &= (~_TRSTS_BSY);			/* to idle state */
		}
		ctrl->event1 = 0;
	}

	/* accept send request */
	if ((ctrl->status1 & _TRSTS_BSY) == 0 &&
		(RAU_SND_SEQ_FLG_NORMAL == RAUhost_GetSndSeqFlag()) &&
		(RAU_SND_SEQ_FLG_NORMAL == RAUhost_GetRcvSeqFlag())) {
		ctrl->status2 |= ctrl->event2;
		ctrl->event2 = 0;

		if (ctrl->status2) {
		/* request exist */
			if ((ctrl->status1 & (_TRSTS_STP|_TRSTS_PEND)) == 0) {
			/* no stop & no pending */
				ctrl->status1 |= _TRSTS_BSY;		/* requesting data send */
				NTNET_Snd_Data61_R(ctrl->status2, 1);
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: transmission of NT-NET number of parked cars data
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET Web用駐車台数データ送信
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntautoSendCtrl_pcarsWeb(void)
{
	NTNET_AUTO_TRANCTRL		*ctrl = &ntTranCtrl;

	if ((ctrl->status1 & (_TRSTS_DMNT|_TRSTS_PWON)) != 0) {
		return;
	}

	if(( NT_pcars_timer == 0 )){				// Timeout ?(Y)
		Make_Log_ParkCarNum();
		NT_pcars_timer = -1;
// MH322914 (s) kasiyama 2016/07/11 満空状態を変更しても駐車台数データ(ID58)が送信されない[共通バグNo.1227](MH341106)
//		if(memcmp(&ParkCar_data.CMN_DT, &ParkCar_data_Bk.CMN_DT, sizeof(ParkCar_data_Bk.CMN_DT)) != 0){
		if((memcmp(&ParkCar_data.CMN_DT, &ParkCar_data_Bk.CMN_DT, sizeof(ParkCar_data_Bk.CMN_DT)) != 0)
			|| (memcmp(&ParkCar_data.Full, &ParkCar_data_Bk.Full, sizeof(ParkCar_data_Bk.Full)) != 0)) {
// MH322914 (e) kasiyama 2016/07/11 満空状態を変更しても駐車台数データ(ID58)が送信されない[共通バグNo.1227](MH341106)
			memcpy(&ParkCar_data_Bk, &ParkCar_data, sizeof(ParkCar_log));
			Log_regist( LOG_PARKING );
		}
		else{
			memset( &ParkCar_data, 0, sizeof(ParkCar_data));
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send number of parked cars at appointed time
 *[]----------------------------------------------------------------------[]
 *| summary	: 駐車台数テーブル送信（定時刻）
 *| return	: 1:send
 *[]----------------------------------------------------------------------[]*/
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//static	int		send_parked_cars1(struct clk_rec *now)
int		send_parked_cars1(struct clk_rec *now)
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
{
	int		at_time;
	int		i, j;
	uchar	hour, min;
	ushort	interval;//定時送信では使用しない引数であるがダミーで必要
	/* check current setting 1st */
	at_time = (search_match_time(now) >= 0)? 1 : 0;

	if (at_time == 0 && wup_flag) {
	/* after wake-up, check last send time */
		struct clk_rec		tmp = ntTranTime.last1;
	/* NOTE:
		前回送信日時には復電後の送信も含まれる
		この場合送信日時はパターンにマッチしないので、時間範囲で近似する
	*/
		i = search_time_range(&tmp, 0, 1, &hour, &min, &interval);		/* get last pattern */
		if (i >= 0) {
		/* get next send time */
			j = get_next_pattern(i, 0, &hour, &min, &interval);
			tmp.hour = hour;
			tmp.minu = min;
			if (j <= i) {
			/* passed one day */
				next_day(&tmp);
			}
			at_time = is_expired(now, &tmp);
		}
		else {
			_DISCARD(ntTranTime.last1);			/* invalid setting */
		}
	}
	return at_time;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send number of parked cars at appointed time & range
 *[]----------------------------------------------------------------------[]
 *| summary	: 駐車台数テーブル送信（定間隔）
 *| return	: 1:send
 *[]----------------------------------------------------------------------[]*/
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//static	int		send_parked_cars2(struct clk_rec *now)
int		send_parked_cars2(struct clk_rec *now)
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
{
	int		at_time;
	int		i, j;
	uchar	hour, min;
	ushort	interval;
	ushort	time0, time1;

	at_time = 0;	

	/* check current setting 1st */
	i = search_time_range(now, 1, 0, &hour, &min, &interval);
	if (i >= 0) {
		time0 = (now->hour*60) + now->minu;
		time1 = (hour*60) + min;
		if (!((time0-time1)%interval)) {
			at_time = 1;			/* match time */
		}
	}

	if (at_time == 0 && wup_flag) {
	/* after wake-up, check last send time */
		struct clk_rec		tmp = ntTranTime.last1;
		i = search_time_range(&tmp, 1, 0, &hour, &min, &interval);	/* get last pattern */
		if (i >= 0) {
			time0 = tmp.hour * 60 + tmp.minu;
			time1 = hour * 60 + min;		/* = target time for next send using current interval */
			if (time0 >= time1) {
				time1 += (((time0-time1)/interval)+1)*interval;
			}
			j = get_next_pattern(i, 0, &hour, &min, &interval);		/* get next pattern no. */
			if(j != 0){//get_next_patternで得たパターン設定がパターン1(j=0)でないときのみ実行する
				time0 = hour * 60 + min;		/* target time using next pattern */
				if (time0 < time1) {
					time1 = time0;				/* choose early time */
				}
			}
			tmp.hour = (uchar)(time1 / 60);
			tmp.minu = (uchar)(time1 % 60);
			at_time = is_expired(now, &tmp);
		}
		else {
		/* NOTE:
			前回送信日時には復電後の送信も含まれる
			送信が無効な時間帯
				パターン１が1:00からで0:30に復電して送信した・・・など
			が設定できるので、それに対応
		*/
			i = search_time_range(&tmp, 1, 1, &hour, &min, &interval);	/* = last valid pattern no. */
			if (i >= 0) {
				j = get_next_pattern(i, 0, &hour, &min, &interval);		/* = 1st valid pattern no. */
				tmp.hour = hour;
				tmp.minu = min;
				at_time = is_expired(now, &tmp);
			}
			else {
				_DISCARD(ntTranTime.last1);			/* invalid setting */
			}
		}
	}
	return at_time;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: search match time(acending order)
 *[]----------------------------------------------------------------------[]
 *| summary	: 時刻が一致するものを捜す
 *| return	: pattern no.
 *[]----------------------------------------------------------------------[]*/
static	int		search_match_time(struct clk_rec *clk)
{
	int		i;
	short	adr;
	ushort	time = (clk->hour*100)+clk->minu;

	for (i = 0; i < 12; i++) {
		adr = 71+(i<<1);
		if (time == CPrmSS[S_NTN][adr]) {
			return i;			/* at appointed time */
		}
	}

	return -1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: search time range(decending order)
 *[]----------------------------------------------------------------------[]
 *| summary	: 時刻が含まれるものを捜す
 *| return	: pattern no.
 *[]----------------------------------------------------------------------[]*/
static	int		search_time_range(struct clk_rec *clk, int interval, int around,
								uchar *phour, uchar *pmin, ushort *pint)
{
	int		i, j;
	short	adr;
	ushort  time = (clk->hour*100)+clk->minu;

	j = -1;
	for (i = 11; i >= 0; i--) {
		adr = 71+(i<<1);
		*phour = (uchar)(CPrmSS[S_NTN][adr]/100);
		*pmin  = (uchar)(CPrmSS[S_NTN][adr]%100);
		*pint  = (ushort)(60*(CPrmSS[S_NTN][adr+1]/100));//時間を分に換算
		*pint  = *pint + (ushort)(CPrmSS[S_NTN][adr+1]%100);//分を加算
		if (*phour >= 24 || *pmin >= 60){
			continue;		/* invalid */
		}
		if (interval) {
			if (*pint == 0)
				continue;	/* invalid */
		}
		if (time >= (ushort)CPrmSS[S_NTN][adr]) {
			return i;			/* at appointed time */
		}
		if (around && j < 0)
			j = i;				/* if go around, choose last pattern */
	}
	return j;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: get next pattern
 *[]----------------------------------------------------------------------[]
 *| summary	: 次のパターンを取得する
 *| return	: pattern no.
 *[]----------------------------------------------------------------------[]*/
static	int		get_next_pattern(int cur, int interval,
								uchar *phour, uchar *pmin, ushort *pint)
{
	int		i;
	short	adr;

	for (i = 0; i < 12; i++) {
		cur++;
		if (cur >= 12)
			cur = 0;
		adr = 71+(cur<<1);
		*phour = (uchar)_REF_NTN(adr, 3, 2);
		*pmin  = (uchar)_REF_NTN(adr, 5, 2);
		*pint  = (ushort)(60*(CPrmSS[S_NTN][adr+1]/100));//時間を分に換算
		*pint  = *pint + (ushort)(CPrmSS[S_NTN][adr+1]%100);//分を加算
		if (*phour >= 24 || *pmin >= 60){
			continue;		/* invalid */
		}
		if (interval) {
			if (*pint == 0)
				continue;	/* invalid */
		}
		return cur;			/* at appointed time */
	}
	return -1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: next day
 *[]----------------------------------------------------------------------[]
 *| summary	: 日付を１日進める
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	next_day(struct clk_rec *clk)
{
	if (clk->date < medget((short)clk->year, (short)clk->mont)) {
		clk->date++;
	}
	else {
		clk->date = 1;
		if (clk->mont < 12) {
			clk->mont++;
		}
		else {
			clk->mont = 1;
			clk->year++;
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: check appointed time is expired
 *[]----------------------------------------------------------------------[]
 *| summary	: 予定時刻を過ぎたか？
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	int	is_expired(struct clk_rec *now, struct clk_rec *app)
{
	if (now->year > app->year){
		 return 1;//現在年より予定年が過去
	}
	if (now->mont > app->mont){
		 return 1;//現在年より予定年が過去
	}
	if (now->date > app->date){
		 return 1;//現在日より予定日が過去
	}
	if (now->year < app->year){
		 return 0;//予定年より現在年が過去
	}
	if (now->mont < app->mont){
		 return 0;//予定年より現在年が過去
	}
	if (now->date < app->date){
		 return 0;//予定日より現在日が過去
	}
//以下年月日が一致している場合で時分で比較する
	if ((now->hour*60)+now->minu >= (app->hour*60)+app->minu){
		 return 1;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send number of parked cars at appointed time
 *[]----------------------------------------------------------------------[]
 *| summary	: 定時刻テーブル送信要求判定(バッチ送信用)
 *| return	: 1:send
 *[]----------------------------------------------------------------------[]*/
static	int		at_batch_time_teiji(struct clk_rec *now)
{
	int		at_time;
	int		i, j;
	uchar	hour, min;
	ushort	interval;//定時送信では使用しない引数であるがダミーで必要
	/* check current setting 1st */
	at_time = (search_match_time_at_batch(now) >= 0)? 1 : 0;

	if (at_time == 0 && wup_flag) {
	/* after wake-up, check last send time */
		struct clk_rec		tmp = ntTranTime.last0;
	/* NOTE:
		前回送信日時には復電後の送信も含まれる
		この場合送信日時はパターンにマッチしないので、時間範囲で近似する
	*/
		i = search_time_range_at_batch(&tmp, 0, 1, &hour, &min, &interval);		/* get last pattern */
		if (i >= 0) {
		/* get next send time */
			j = get_next_pattern_at_batch(i, 0, &hour, &min, &interval);
			tmp.hour = hour;
			tmp.minu = min;
			if (j <= i) {
			/* passed one day */
				next_day(&tmp);
			}
			at_time = is_expired(now, &tmp);
		}
		else {
			_DISCARD(ntTranTime.last0);			/* invalid setting */
		}
	}
	return at_time;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: send number of parked cars at appointed time & range
 *[]----------------------------------------------------------------------[]
 *| summary	: 定間隔テーブル送信要求判定(バッチ送信用)
 *| return	: 1:send
 *[]----------------------------------------------------------------------[]*/
static	int		at_batch_time_teikankaku(struct clk_rec *now)
{
	int		at_time;
	int		i, j;
	uchar	hour, min;
	ushort	interval;
	ushort	time0, time1;

	at_time = 0;	

	/* check current setting 1st */
	i = search_time_range_at_batch(now, 1, 0, &hour, &min, &interval);
	if (i >= 0) {
		time0 = (now->hour*60) + now->minu;
		time1 = (hour*60) + min;
		if (!((time0-time1)%interval)) {
			at_time = 1;			/* match time */
		}
	}

	if (at_time == 0 && wup_flag) {
	/* after wake-up, check last send time */
		struct clk_rec		tmp = ntTranTime.last0;
		i = search_time_range_at_batch(&tmp, 1, 0, &hour, &min, &interval);	/* get last pattern */
		if (i >= 0) {
			time0 = tmp.hour * 60 + tmp.minu;
			time1 = hour * 60 + min;		/* = target time for next send using current interval */
			if (time0 >= time1) {
				time1 += (((time0-time1)/interval)+1)*interval;
			}
			j = get_next_pattern_at_batch(i, 0, &hour, &min, &interval);		/* get next pattern no. */
			if(j != 0){//get_next_pattern_at_batchで得たパターン設定がパターン1(j=0)でないときのみ実行する
				time0 = hour * 60 + min;		/* target time using next pattern */
				if (time0 < time1) {
					time1 = time0;				/* choose early time */
				}
			}
			tmp.hour = (uchar)(time1 / 60);
			tmp.minu = (uchar)(time1 % 60);
			at_time = is_expired(now, &tmp);
		}
		else {
		/* NOTE:
			前回送信日時には復電後の送信も含まれる
			送信が無効な時間帯
				パターン１が1:00からで0:30に復電して送信した・・・など
			が設定できるので、それに対応
		*/
			i = search_time_range_at_batch(&tmp, 1, 1, &hour, &min, &interval);	/* = last valid pattern no. */
			if (i >= 0) {
				j = get_next_pattern_at_batch(i, 0, &hour, &min, &interval);		/* = 1st valid pattern no. */
				tmp.hour = hour;
				tmp.minu = min;
				at_time = is_expired(now, &tmp);
			}
			else {
				_DISCARD(ntTranTime.last0);			/* invalid setting */
			}
		}
	}
	return at_time;

}
/*[]----------------------------------------------------------------------[]
 *|	name	: search match time_at_batch(acending order)
 *[]----------------------------------------------------------------------[]
 *| summary	: 時刻が一致するものを捜す(バッチ送信用)
 *| return	: pattern no.
 *[]----------------------------------------------------------------------[]*/
static	int		search_match_time_at_batch(struct clk_rec *clk)
{
	int		i;
	short	adr;
	ushort	time = (clk->hour*100)+clk->minu;

	for (i = 0; i < 12; i++) {
		adr = 95+(i<<1);
		if (time == CPrmSS[S_NTN][adr]) {
			return i;			/* at appointed time */
		}
	}

	return -1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: search time range(decending order)
 *[]----------------------------------------------------------------------[]
 *| summary	: 時刻が含まれるものを捜す(バッチ送信用)
 *| return	: pattern no.
 *[]----------------------------------------------------------------------[]*/
static	int		search_time_range_at_batch(struct clk_rec *clk, int interval, int around,
								uchar *phour, uchar *pmin, ushort *pint)
{
	int		i, j;
	short	adr;
	ushort  time = (clk->hour*100)+clk->minu;

	j = -1;
	for (i = 11; i >= 0; i--) {
		adr = 95+(i<<1);
		*phour = (uchar)(CPrmSS[S_NTN][adr]/100);
		*pmin  = (uchar)(CPrmSS[S_NTN][adr]%100);
		*pint  = (ushort)(60*(CPrmSS[S_NTN][adr+1]/100));//時間を分に換算
		*pint  = *pint + (ushort)(CPrmSS[S_NTN][adr+1]%100);//分を加算
		if (*phour >= 24 || *pmin >= 60)
			continue;		/* invalid */
		if (interval) {
			if (*pint == 0)
				continue;	/* invalid */
		}
		if (time >= (ushort)CPrmSS[S_NTN][adr]) {
			return i;			/* at appointed time */
		}
		if (around && j < 0)
			j = i;				/* if go around, choose last pattern */
	}
	return j;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: get next pattern_at_batch
 *[]----------------------------------------------------------------------[]
 *| summary	: 次のパターンを取得する(バッチ送信用)
 *| return	: pattern no.
 *[]----------------------------------------------------------------------[]*/
static	int		get_next_pattern_at_batch(int cur, int interval,
								uchar *phour, uchar *pmin, ushort *pint)
{
	int		i;
	short	adr;

	for (i = 0; i < 12; i++) {
		cur++;
		if (cur >= 12)
			cur = 0;
		adr = 95+(cur<<1);
		*phour = (uchar)_REF_NTN(adr, 3, 2);
		*pmin  = (uchar)_REF_NTN(adr, 5, 2);
		*pint  = (ushort)(60*(CPrmSS[S_NTN][adr+1]/100));//時間を分に換算
		*pint  = *pint + (ushort)(CPrmSS[S_NTN][adr+1]%100);//分を加算
		if (*phour >= 24 || *pmin >= 60)
			continue;		/* invalid */
		if (interval) {
			if (*pint == 0)
				continue;	/* invalid */
		}
		return cur;			/* at appointed time */
	}
	return -1;
}

void NTNET_RAUResult_Send(uchar systemID, uchar dataType, uchar result)
{
	if (systemID == REMOTE_SYSTEM) {
		if (z_NtRemoteAutoManual == REMOTE_AUTO) {
			ntautoPostResult(dataType, (uchar)result);
		} else {
		// メンテ通知
			IBKCtrl_SetRcvData_manu(dataType, (uchar)result);
		}
	}
}
