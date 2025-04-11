// MH810100(S) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
//[]----------------------------------------------------------------------[]
///	@brief			パケット通信バッファ管理モジュール
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pkt_buffer.c
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"message.h"
#include	"ope_def.h"
#include	"pkt_com.h"


#define	PKT_BUFF_SIZ	PKT_CMD_MAX			// バッファサイズ＝データ部の最大サイズ

// ソケットバッファー
typedef	struct {
	ushort	count;
	ushort	total;
	ushort	rp;
	ushort	wp;
	uchar	buff[PKT_BUFF_SIZ];
} PKT_RCVSND_BUFF;

static PKT_RCVSND_BUFF	g_snd_buf;
static PKT_RCVSND_BUFF	g_rcv_buf;
#define snd_buf			g_snd_buf
#define rcv_buf			g_rcv_buf

#define	PKT_RSND_BUFF_SIZ	PKT_CMD_MAX-2	// データ部の最大サイズ－電文長

// パケット再送信バッファー
typedef struct {
	ushort	count;
	ushort	total;
	ushort	rp;
	ushort	wp;
	uchar	buff[PKT_RSND_BUFF_SIZ];
} PKT_RSND_BUFF;

static PKT_RSND_BUFF	g_rsnd_add_cmd;
#define rsnd_add_cmd	g_rsnd_add_cmd


static void LockPKTSnd( void );
static void UnLockPKTSnd( void );
static void LockPKTRcv( void );
static void UnLockPKTRcv( void );

//[]----------------------------------------------------------------------[]
///	@brief			バッファイニシャライズ
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			なし
///	@note			コマンド送信バッファとコマンド受信バッファをクリアします。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PKTbuf_init( void )
{
	memset( &snd_buf, 0, sizeof(snd_buf) );
	memset( &rsnd_add_cmd, 0, sizeof(rsnd_add_cmd) );
	memset( &rcv_buf, 0, sizeof(rcv_buf) );
}

//[]----------------------------------------------------------------------[]
///	@brief			最古のコマンドを再送用バッファから削除する
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			コマンドサイズ(コマンド長+コマンドデータ長)
///	@attention		コマンドはコマンド長(2byte)とコマンドデータで構成されます。
///	@note			バッファリングされているコマンドの中で最古のものを削除する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void PKTbuf_DelReSendCommand( void )
{
	ushort	rp;
	ushort	size, len;
	PKT_RSND_BUFF	*cmd_buf;

	cmd_buf = &rsnd_add_cmd;

	if( cmd_buf->count == 0 ){
		return;
	}

	if( --cmd_buf->count == 0 ){
		// buffer reset
		cmd_buf->total = 0;
		cmd_buf->rp =  0;
		cmd_buf->wp = 0;
		return;
	}

	rp = cmd_buf->rp;
	size = *(ushort*)&cmd_buf->buff[rp];
	len = size;
	rp += (2 + len);
	if( rp >= PKT_RSND_BUFF_SIZ ){
		rp -= PKT_RSND_BUFF_SIZ;
	}
	cmd_buf->rp = rp;
	cmd_buf->total -= (2 + len);
}

//[]----------------------------------------------------------------------[]
///	@brief			送信するコマンドデータを再送用バッファにセットする
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: セットするコマンドデータ
///	@param[in]		len		: コマンド長
///	@return			1:OK<br>
///					0:NG
///	@attention		バッファがオーバーフローするときは最古のデータから破棄して<br>
///					最新データを保存します。
///	@note			コネクション回復後に再送するコマンドを管理します。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
BOOL PKTbuf_SetReSendCommand( const uchar *data, ushort len )
{
	ushort	wp;
	ushort	size;
	PKT_RSND_BUFF	*cmd_buf;

	cmd_buf = &rsnd_add_cmd;

	size = len;
	while ((cmd_buf->total + len) > PKT_RSND_BUFF_SIZ) {		// overflow
		PKTbuf_DelReSendCommand();
	}

	wp = cmd_buf->wp;
	if ((wp+size) <= PKT_RSND_BUFF_SIZ) {
		memcpy(&cmd_buf->buff[wp], data, (size_t)size);
	} else {
		ushort	remain = PKT_RSND_BUFF_SIZ - wp;
		// バファの終端までセット
		memcpy(&cmd_buf->buff[wp], data, (size_t)remain);
		size -= remain;
		// 残りはバッファ先頭以降にセット
		memcpy(&cmd_buf->buff[0], &data[remain], (size_t)size);
	}

	wp += len;
	if (wp >= PKT_RSND_BUFF_SIZ) {
		wp -= PKT_RSND_BUFF_SIZ;
	}
	cmd_buf->wp = wp;
	cmd_buf->total += len;
	cmd_buf->count++;

	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			コマンドを再送用バッファからリードする
//[]----------------------------------------------------------------------[]
///	@param[in]		num		: ｎ番目のコマンド
///	@param[out]		*data	: リードしたコマンド
///	@return			コマンドサイズ(コマンド長+コマンドデータ長)
///	@attention		コマンドはコマンド長(2byte)とコマンドデータで構成されます。
///	@note			バッファリングされているコマンドの中で最古のものを基準に<br>
///					ｎ番目のコマンドをリードする。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort PKTbuf_ReadReSendCommand( ushort num, uchar *data )
{
	ushort	rp;
	ushort	size = 0, len;
	PKT_RSND_BUFF	*cmd_buf;
	ushort	count;

	cmd_buf = &rsnd_add_cmd;

	if( cmd_buf->count <= num ){
		return 0;
	}
	rp = cmd_buf->rp;
	for( count = 0; count <= num; count++ ){
		size = *(ushort*)&cmd_buf->buff[rp];
		size += 2;
		len = size;
		if (count == num) {
			if ((rp+size) <= PKT_RSND_BUFF_SIZ) {
				memcpy(data, &cmd_buf->buff[rp], (size_t)size);
			}
			else {
				ushort	remain = PKT_RSND_BUFF_SIZ - rp;
				// バファの終端まで読み出し
				memcpy(data, &cmd_buf->buff[rp], (size_t)remain);
				// 残りはバッファ先頭から読み出し
				memcpy(&data[remain], &cmd_buf->buff[0], (size_t)(size - remain));
			}
			break;
		}

		rp += len;
		if (rp >= PKT_RSND_BUFF_SIZ) {
			rp -= PKT_RSND_BUFF_SIZ;
		}
	}

	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			再送用バッファをクリアする
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/21<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PKTbuf_ClearReSendCommand( void )
{
	PKT_RSND_BUFF	*cmd_buf;

	cmd_buf = &rsnd_add_cmd;

// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:604)対応
//	memset(&cmd_buf->count, 0, sizeof(PKT_RSND_BUFF));
	memset(cmd_buf, 0, sizeof(PKT_RSND_BUFF));
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:604)対応
}

//[]----------------------------------------------------------------------[]
///	@brief			再送中コマンドをチェックする
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
///	@attention		
///	@note			再送用バッファの先頭にセットされているコマンドIDとサブ
///					コマンドIDを返します
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/25<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ulong PKTbuf_CheckReSendCommand( void )
{
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} stID;
	ulong	ID_SUBID;

	stID.uc.high = rsnd_add_cmd.buff[2];
	stID.uc.low  = rsnd_add_cmd.buff[3];
	ID_SUBID = stID.us << 16;

	stID.uc.high = rsnd_add_cmd.buff[4];
	stID.uc.low  = rsnd_add_cmd.buff[5];
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:606)対応
//	ID_SUBID |= stID.us;
	ID_SUBID |= (ulong)stID.us;
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:606)対応

	return ID_SUBID;
}

//[]----------------------------------------------------------------------[]
///	@brief			送信するコマンドデータをバッファにセットする
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: セットするコマンドデータ
///	@param[in]		len		: コマンド長
///	@return			TRUE	: OK<br>
///					FALSE	: NG
///	@attention		バッファがオーバーフローするときは書込み対象のデータを破棄する。
///	@note			このバッファにセットされたコマンドはパケット通信タスクが<br>
///					リードしてパケット化します。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
BOOL PKTbuf_SetSendCommand( const uchar *data, ushort len )
{
	int		i = 0;	// ループカウンタ
	ushort	wp;
	ushort	size;

	size = len;

	LockPKTSnd();

	// バッファに入りきらない？
	if( (snd_buf.total + len) > PKT_BUFF_SIZ ){
		// リトライする
		for( i=0; i<5; i++ ){
			// バッファに入りきらない？
			if( (snd_buf.total + len) > PKT_BUFF_SIZ ){
				queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );
				taskchg( IDLETSKNO );
			}else{
				break;
			}
		}
		if( i >= 5) {
			// バッファに入りきらない？
			if( (snd_buf.total + len) > PKT_BUFF_SIZ ){
				err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_BUF_FUL, 2, 0, 0, NULL );
				UnLockPKTSnd();
				return FALSE;		// overflow
			}
		}
	}

	if( PKT_BUFF_SIZ < snd_buf.wp ){	// 送信バッファライトポインタ異常
		// リード/ライトポインタをクリアして今回のコマンドを書き込む
		snd_buf.rp = 0;
		snd_buf.wp = 0;
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_BUF_WPT, 2, 0, 0, NULL );
	}
	wp = snd_buf.wp;
	wp %= PKT_BUFF_SIZ;

	// 収まる？
	if( (wp+size) <= PKT_BUFF_SIZ ){
		memcpy( &snd_buf.buff[wp], data, (size_t)size );
	}else{
		ushort	remain = PKT_BUFF_SIZ - wp;
		// バファの終端までセット
		memcpy( &snd_buf.buff[wp], data, (size_t)remain );
		size -= remain;
		// 残りはバッファ先頭以降にセット
		memcpy( &snd_buf.buff[0], &data[remain], (size_t)size );
	}

	wp += len;
	wp %= PKT_BUFF_SIZ;
	snd_buf.wp = wp;
	snd_buf.total += len;
	snd_buf.count++;
	
	UnLockPKTSnd();

	queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );

	return TRUE;
}

//[]----------------------------------------------------------------------[]
///	@brief			コマンドをバッファからリードする
//[]----------------------------------------------------------------------[]
///	@param[out]		*data	: リードしたコマンド
///	@param[in]		limit	: 制限サイズ(0～PKT_CMD_MAX)
///	@return			コマンドサイズ(コマンド長+コマンドデータ長)
///	@attention		コマンドはコマンド長(2byte)とコマンドデータで構成されます。
///	@note			バッファリングされているコマンドの中で最古のものをリードする。<br>
///					コマンドが制限サイズより大きい場合はリードしない。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort PKTbuf_ReadSendCommand( uchar *data, ushort limit )
{
	ushort	rp;
	ushort	size, len;

	LockPKTSnd();

	// バッファにアイテムなし？
	if( snd_buf.count == 0 ){
		UnLockPKTSnd();
		return 0;
	}

	if( PKT_BUFF_SIZ < snd_buf.rp ){	// 送信バッファリードポインタ異常
		// リード/ライトポインタをクリアして読み込まずに戻る
		snd_buf.rp = 0;
		snd_buf.wp = 0;
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_BUF_RPT, 2, 0, 0, NULL );
		return 0;
	}
	rp = snd_buf.rp;
	size = *(ushort*)&snd_buf.buff[rp];
	size += 2;
	if( size > limit ){
		UnLockPKTSnd();
		return 0;
	}

	len = size;
	rp %= PKT_BUFF_SIZ;
	if( (rp+size) <= PKT_BUFF_SIZ ){
		memcpy( data, &snd_buf.buff[rp], (size_t)size );
	}
	else{
		ushort	remain = PKT_BUFF_SIZ - rp;
		// バファの終端まで読み出し
		memcpy( data, &snd_buf.buff[rp], (size_t)remain );
		// 残りはバッファ先頭から読み出し
		memcpy( &data[remain], &snd_buf.buff[0], (size_t)(size - remain) );
	}

	rp += len;
	rp %= PKT_BUFF_SIZ;
	snd_buf.rp = rp;
	snd_buf.total -= len;
	// 今回のでバッファが空になった？
	if( --snd_buf.count == 0 ){
		// buffer reset
		snd_buf.total = 0;
		snd_buf.rp = 0;
		snd_buf.wp = 0;
	}
	UnLockPKTSnd();

	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			受信したコマンドデータをバッファにセットする
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: セットするコマンドデータ
///	@param[in]		len		: コマンド長
///	@return			1:OK<br>
///					0:NG
///	@attention		バッファがオーバーフローするときは書込み対象のデータを破棄する。
///	@note			このバッファにセットされたコマンドはメールで<br>
///					オペレーションタスクに通知します。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2009/09/25<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
BOOL PKTbuf_SetRecvCommand(const uchar *data, ushort len)
{
	ushort	wp;
	ushort	size;

	LockPKTRcv();

	size = len;
	if ((rcv_buf.total + 2 + len) > PKT_BUFF_SIZ) {
		UnLockPKTRcv();
		return 0;		// overflow
	}
	wp = rcv_buf.wp;
	*(ushort*)&rcv_buf.buff[wp] = size;		// 必ず保存できる

	wp += 2;
	wp %= PKT_BUFF_SIZ;
	if ((wp+size) <= PKT_BUFF_SIZ) {
		memcpy(&rcv_buf.buff[wp], data, (size_t)size);
	} else {
		ushort	remain = PKT_BUFF_SIZ - wp;
		// バファの終端までセット
		memcpy(&rcv_buf.buff[wp], data, (size_t)remain);
		size -= remain;
		// 残りはバッファ先頭以降にセット
		memcpy(&rcv_buf.buff[0], &data[remain], (size_t)size);
	}

	wp += len;
	wp %= PKT_BUFF_SIZ;
	rcv_buf.wp = wp;
	rcv_buf.total += (2+len);
	rcv_buf.count++;

	UnLockPKTRcv();

	return 1;
}


//[]----------------------------------------------------------------------[]
///	@brief			外部から受信したコマンドをバッファからリードする
//[]----------------------------------------------------------------------[]
///	@param[out]		*data	: リードしたコマンド
///	@return			コマンドサイズ(コマンド長+コマンドデータ長)
///	@attention		オペレーションタスクは受信通知をメールで受ける度にリードすること。
///	@note			バッファリングされているコマンドの中で最古のものをリードする。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2009/09/30<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
ushort PKTbuf_ReadRecvCommand(uchar *data)
{
	ushort	rp;
	ushort	size, len;

	LockPKTRcv();

	if (rcv_buf.count == 0) {
		UnLockPKTRcv();
		return 0;
	}
	rp = rcv_buf.rp;
	size = *(ushort*)&rcv_buf.buff[rp];
	len = size;
	rp += 2;
	rp %= PKT_BUFF_SIZ;
	if ((rp+size) <= PKT_BUFF_SIZ) {
		memcpy(data, &rcv_buf.buff[rp], (size_t)size);
	}
	else {
		ushort	remain = PKT_BUFF_SIZ - rp;
		// バファの終端まで読み出し
		memcpy(data, &rcv_buf.buff[rp], (size_t)remain);
		// 残りはバッファ先頭から読み出し
		memcpy(&data[remain], &rcv_buf.buff[0], (size_t)(size - remain));
	}
	rp += len;
	rp %= PKT_BUFF_SIZ;
	rcv_buf.rp = rp;
	rcv_buf.total -= (2 + len);
	if (--rcv_buf.count == 0) {
		// buffer reset
		rcv_buf.total =
		    rcv_buf.rp =
		        rcv_buf.wp = 0;
	}

	UnLockPKTRcv();

	return(size);
}

static void LockPKTSnd( void )
{
//	wai_sem(ID_PKT_SND_BUFF_SEM);
}

static void UnLockPKTSnd( void )
{
//	sig_sem(ID_PKT_SND_BUFF_SEM);
}

static void LockPKTRcv( void )
{
//	wai_sem(ID_PKT_RCV_BUFF_SEM);
}

static void UnLockPKTRcv( void )
{
//	sig_sem(ID_PKT_RCV_BUFF_SEM);
}
// MH810100(E) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
