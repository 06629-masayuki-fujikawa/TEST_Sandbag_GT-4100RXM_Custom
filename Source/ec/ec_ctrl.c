/*[]----------------------------------------------------------------------[]*/
/*| EC CRW-MJA R/W Communication Control                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : G.So                                                     |*/
/*| Date        : 2019.02.07                                               |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
#include	<stdio.h>
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"suica_def.h"
#include	"ec_def.h"
#include	"prm_tbl.h"

/*[]----------------------------------------------------------------------[]*/
/*| 決済リーダ制御部 部番                                                     |*/
/*[]----------------------------------------------------------------------[]*/
const	ver_rec		VERSNO_EC_COMM = {
// MH321800(S) 部番変更
//	'0',' ','G','G','1','1','6','8','0','0'				// 決済リーダ対応部番
	'0',' ','M','H','3','4','3','7','0','0'				// 決済リーダ対応部番
// MH321800(E) 部番変更
};

// MH810103 GG119202(S) 起動シーケンス不具合修正
extern	int		ModuleStartFlag;
// MH810103 GG119202(E) 起動シーケンス不具合修正

/*[]----------------------------------------------------------------------[]*/
/*| CRW-MJA Initialize                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : crwmja_init                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void ec_init( void )
{
	short	ct;
//MH810103  GG119202(S) 起動シーケンス不具合修正
//// GG119200(S) 起動シーケンス完了条件見直し
////	short	max = 14;
//	short	max = 15;
//// GG119200(E) 起動シーケンス完了条件見直し
	short	max = 4;
// MH810103 GG119202(E) 起動シーケンス不具合修正

// setup
	jvma_setup.mdl = ERRMDL_EC;
	jvma_setup.recv = ec_recv;

// init.
	ec_flag_clear(1);								// 関連フラグ初期化

	suica_errst = 0;
	memset( suica_err,0,sizeof(suica_err) );
	memset( &suica_work_time, 0, sizeof( suica_work_time ));

	Suica_Rec.snd_kind = 0;
	Ope_Suica_Status = 0;							// Ope側からの送信ｽﾃｰﾀｽ初期化
	memset( &Suica_Snd_Buf,0,sizeof( SUICA_SEND_BUFF ));

// initial seq.
	for( ct=0;ct<max; ct++ ){
		switch( ct ){
		case 0: 									/* Stand-by */
			Suica_Rec.Com_kind = 0;					
			Suica_Rec.Status = STANDBY_BEFORE;
			break;
		case 1: 									/* All req. */
			Suica_Rec.Com_kind = 1;					
			Suica_Rec.Status = STANDBY_SND_AFTER;
			break;
		case 3:										/* Out req. */
			if( Suica_Rec.Status == DATA_RCV ){
				Ec_Data_Snd( S_INDIVIDUAL_DATA,&CLK_REC );
				jvma_command_set();
			}
			break;
// MH810103 GG119202(S) 起動シーケンス不具合修正
//		case 12:									/* Out req. */
//			if( Suica_Rec.Status == DATA_RCV ){
//// GG119200(S) 起動シーケンス完了条件見直し
//				if( !isEcBrandStsDataRecv() ) {		// 全ブランド状態データ未受信
//					break;
//				}
//// GG119200(E) 起動シーケンス完了条件見直し
//				SetEcVolume();
//				Ec_Data_Snd( S_VOLUME_CHG_DATA, &VolumeTbl );
//				jvma_command_set();
//			}
//			break;
//		case 2: 									/* Input req. */
//		case 4:										/* wait for brand status 1-01 */
//		case 6:										/* wait for brand status 1-02 */
//		case 8:										/* wait for brand status 2-01 */
//		case 10:									/* wait for brand status 2-02 */
//		case 13:									/* wait for volume result */
		case 2: 									/* Input req. */
// MH810103 GG119202(E) 起動シーケンス不具合修正
			Suica_Rec.Com_kind = 2;
			break;
// MH810103 GG119202(S) 起動シーケンス不具合修正
//													/* Out req. */
//		case 5:										/* send brand setting 1-01 */
//		case 7:										/* send brand setting 1-02 */
//		case 9:										/* send brand setting 2-01 */
//		case 11:									/* send brand setting 3-02 */
//			// 通常はec_recv()からSUICA_EVTがopetaskに通知されて
//			// Ope_Ec_Event()が呼ばれるが、保険のためこのタイミングでも
//			// Ope_Ec_Event()を呼んで受信処理を行う
//			Ope_Ec_Event(0, 0);
//			jvma_command_set();
//			if(ct == 7 && ECCTL.brand_num <= EC_BRAND_MAX) {
//				// ブランドネゴシエーション１完了時にブランド状態データ１で通知された
//				// ブランド数が10件を超える場合のみブランドネゴシエーション２を実施
//				// ブランド数が10件未満の場合は音量時間帯設定データ(DC=31H)を送信するため
//				// ctに+4して12とする(for文で+1されるため、ここでは+4とする)
//				ct += 4;
//			}
//			break;
//// GG119200(S) 起動シーケンス完了条件見直し
//		case 14:									// 音量変更結果データ受信を先に実施
//			Ope_Ec_Event(0, 0);
//			break;
//// GG119200(E) 起動シーケンス完了条件見直し
// MH810103 GG119202(E) 起動シーケンス不具合修正

		default:
			break;
		}
		if( 4 == jvma_act() ){						// ACK4 receive
			// 初期化シーケンス中はACK4受信エラーを登録しない
			return;									// 次のPOL契機でスタンバイ送信(イニシャル未完)
		}
		suica_errst = suica_err[0];
		if( suica_errst )
			return;
// MH810103 GG119202(S) JVMAリセット処理不具合
//		cnwait( 5 );								/* 100msec wait */
		cnwait( 2 );								/* 40msec wait */
// MH810103 GG119202(E) JVMAリセット処理不具合
// MH810103 GG119202(S) 起動シーケンス不具合修正
		if (ModuleStartFlag != 0) {
			Ope_Ec_Event(0, 0);
		}
// MH810103 GG119202(E) 起動シーケンス不具合修正
	}

// MH810103 GG119202(S) 起動シーケンス不具合修正
//// GG119200(S) 起動シーケンス完了条件見直し
////	if( !Suica_Rec.Data.BIT.BRAND_STS_RCV ){
////		// ブランド状態データを受信してない = 起動シーケンスを行っていないとみなす
////		return;
////	}
//	if( ( !Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.VOL_RES_RCV ) ) {	// 音量変更結果データ未受信
//		return;
//	}
//// GG119200(E) 起動シーケンス完了条件見直し
//
//	Suica_Rec.Data.BIT.INITIALIZE = 1;				/* Finished Initial */
//	ECCTL.phase = EC_PHASE_PAY;						// 初期化中解除
//	ECCTL.step = 0;
	ECCTL.phase = EC_PHASE_BRAND;				// ブランド合わせ中
	ECCTL.step = 0;

	// 最初のブランド状態データ受信待ち
	Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);	// 受信待ちタイマ開始
// MH810103 GG119202(E) 起動シーケンス不具合修正

// MH810103 GG119202(S) 起動シーケンス不具合修正
//	LagCan500ms(LAG500_EC_WAIT_BOOT_TIMER);			// 起動完了待ちタイマ停止
//	// E3210解除
//	err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 0, 0, 0 );
//
//	queset( OPETCBNO, SUICA_INITIAL_END, 0, NULL );	// OpeTaskにSuicaの初期化完了を通知
// MH810103 GG119202(E) 起動シーケンス不具合修正
}

/*[]----------------------------------------------------------------------[]*/
/*| Get Receive Charactor Inf.                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_recv                                              |*/
/*| PARAMETER	 : char *buf    : Receive Buffer Address                   |*/
/*|              : short   siz    : number of Receive Charactor            |*/
/*| RETURN VALUE : short ret;     : 1:ACK1+DATA                            |*/
/*|              : 2:ACK2+DATA                                             |*/
/*|              : 3:ACK3+DATA                                             |*/
/*|              : 4:ACK4                                                  |*/
/*|              : 5:ACK5                                                  |*/
/*|              : 6:NAK                                                   |*/
/*|              : 7:ETC. Char.                                            |*/
/*|              : 9:LRC ERROR                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short ec_recv( unsigned char *buf,short siz )
{
	short	i, rtn, bc;
	unsigned char	*ack;

	// 決済結果データだけは処理済で無い限り受信情報を消さない
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//	if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// 決済結果ﾃﾞｰﾀを受信し
//		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) ){	// 決済結果ﾃﾞｰﾀ未処理の場合
//		Suica_Rec.suica_rcv_event.BYTE = 0;
//		Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;				// 決済結果データだけは消さない
//	}
//	else if( (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 	// 決済結果ﾃﾞｰﾀを受信し
//			(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 1) ){// 決済結果ﾃﾞｰﾀ処理済みの場合
//		Suica_Rec.suica_rcv_event.BYTE = 0;
//	}
//	else if( Suica_Rec.suica_rcv_event.BIT.SETTSTS_DATA == 1 ){
	if( (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1) && 			// 決済結果ﾃﾞｰﾀを受信し
		(Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0) ){		// 決済結果ﾃﾞｰﾀ未処理の場合
		Suica_Rec.suica_rcv_event.BYTE = 0;
		Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;				// 決済結果データだけは消さない
	}
	else if( (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1) && 	// 決済結果ﾃﾞｰﾀを受信し
			(Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 1) ){	// 決済結果ﾃﾞｰﾀ処理済みの場合
		Suica_Rec.suica_rcv_event.BYTE = 0;
	}
	else if( Suica_Rec.suica_rcv_event.EC_BIT.SETTSTS_DATA == 1 ){
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
	}
	else{
		Suica_Rec.suica_rcv_event.BYTE = 0;
	}
	Suica_Rec.suica_rcv_event_sumi.BYTE = 0;

	rtn = 0;
	ack = (unsigned char *)ack_tbl;
	for( i=1; i<7; i++,ack++){
		if(*buf == *ack) break;
	}

	if(( siz > 1 ) && ( i < 4 )) {		// ACK2,3 + データ電文受信
		buf++;
		bc = (short)(*buf);	/* bc <- Total Byte */
		for( ; ; ){
		    if( siz != (bc+3) ){	/* Recieve Size OK ? YJ */
				ex_errlg( jvma_setup.mdl, ERR_EC_DATANUM, 2, 0 );	// Recv packet size error
				i = 9;			/* Size Error 	*/
				break;
		    }
		    if( (unsigned char)(*(buf+bc+1)) != bcccal( (char*)buf, (short)(bc+1) ) ){ /* LRC Ok ? YJ */
				ex_errlg( jvma_setup.mdl, ERR_EC_DATABCC, 2, 0 );	// BCC error
				i = 9;			/* LRC Error */
				break;
		    }

			if( Suica_Rec.Status == STANDBY_SND_AFTER ){		// ｽﾀﾝﾊﾞｲ送信後のﾃﾞｰﾀ受信のためﾃﾞｰﾀ破棄
				SUICA_RCLR();	/* Recieve Inf. Clear */
				Suica_Rec.Status = DATA_RCV;
				return (i);
			}

			if( i >= 1 && i <= 3 ){
				jvma_setup.log_regist( (uchar*)(buf+(i!=2?1:0)),(ushort)(i!=2?bc:bc+1), 0 );
				buf--;											// 受信データの先頭バイトを指定
				Ec_recv_data_regist( buf,(ushort)(bc+2) );		// 受信キューへ格納
				queset( OPETCBNO, SUICA_EVT, 0, NULL );			// OpeTaskへ受信通知
				Suica_Rec.Status = DATA_RCV;
			}
		    break;
		}
	}else if( siz == 1 ){
		if( *buf == 0x11 ){
			Suica_Rec.Status = DATA_RCV;
		}
	}
	SUICA_RCLR();	/* Recieve Inf. Clear */
	rtn = i;
	return( (short)rtn );
}

//[]----------------------------------------------------------------------[]
///	@brief			受信ﾃﾞｰﾀ登録処理
//[]----------------------------------------------------------------------[]
///	@param[in]		buf		:登録するﾃﾞｰﾀﾎﾟｲﾝﾀ
///	@param[in]		size	:登録するﾃﾞｰﾀｻｲｽﾞ<br>
///	@return			void		:
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019.02.07<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void 	Ec_recv_data_regist(uchar *buf , ushort size)
{
	uchar	*wbuf = suica_rcv_que.rcvdata[suica_rcv_que.writept];					// 今回書込む受信ﾊﾞｯﾌｧﾎﾟｲﾝﾀをｾｯﾄ
	uchar	wsize;																	// 検索用ｴﾘｱ
	ushort	i;																		// ﾙｰﾌﾟ用変数
	ushort	w_counter[3];															// ｵﾌｾｯﾄ編集ｴﾘｱ

	memcpy(w_counter, &suica_rcv_que.readpt, sizeof(w_counter));					// ｵﾌｾｯﾄをﾜｰｸ領域に展開
	if ((w_counter[2] == SUICA_QUE_MAX_COUNT) && (w_counter[1] == w_counter[0])) {	// ﾊﾞｯﾌｧがﾌﾙ状態の場合

		for( i=0; (i<6 && *wbuf != ack_tbl[i]); i++);								// 受信種別(ACK1～NACK)検索

		if( i == 1 ){																// 単体ﾃﾞｰﾀの場合
			if (*(wbuf+2) == S_SETTLEMENT_DATA) {									// 決済結果データ
				EcSettlementPhaseError(wbuf+3, 6);									// ｴﾗｰ登録(E3259)
				if(Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {							// 決済処理中に通信異常が発生？
					queset( OPETCBNO, EC_EVT_DEEMED_SETTLEMENT, 0, 0 );				// opeへ通知
				}
			}
		}else{																		// 複数ﾃﾞｰﾀの場合
			wsize = *(wbuf+1);														// ﾃﾞｰﾀｻｲｽﾞを設定
			wbuf++;																	// ﾃﾞｰﾀﾎﾟｲﾝﾀを更新
			for( i=0; i<wsize; ){													// 全ﾃﾞｰﾀを検索するまでﾙｰﾌﾟ
				if (*(wbuf+2) == S_SETTLEMENT_DATA) {								// 決済結果データ
					EcSettlementPhaseError(wbuf+3, 6);								// ｴﾗｰ登録(E3259)
					if(Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {						// 決済処理中に通信異常が発生？
						queset( OPETCBNO, EC_EVT_DEEMED_SETTLEMENT, 0, 0 );			// opeへ通知
					}
				}
				i+=*(wbuf+1)+1;														// 検索済みのﾃﾞｰﾀｻｲｽﾞを更新
				wbuf+=*(wbuf+1)+1;													// ﾃﾞｰﾀﾎﾟｲﾝﾀを更新し、次のﾃﾞｰﾀを参照
			}
		}

		if( ++w_counter[0] >= SUICA_QUE_MAX_COUNT ){								// 読込みｵﾌｾｯﾄ更新＆次に読込むｵﾌｾｯﾄがMAX越えの場合
			w_counter[0] = 0;														// ｵﾌｾｯﾄを先頭に移動
		}
	}
	memcpy( suica_rcv_que.rcvdata[w_counter[1]], buf, (size_t)size );				// 受信してきたﾃﾞｰﾀを受信ｷｭｰに登録
	if( ++w_counter[1] >= SUICA_QUE_MAX_COUNT ){									// 書込みｵﾌｾｯﾄ更新＆次に読込むｵﾌｾｯﾄがMAX越えの場合
		w_counter[1] = 0;															// ｵﾌｾｯﾄを先頭に移動
	}

	if( w_counter[2] < SUICA_QUE_MAX_COUNT ){										// ﾃﾞｰﾀｶｳﾝﾄがMAXでなければ
		w_counter[2]++;																// ﾃﾞｰﾀｶｳﾝﾄをｲﾝｸﾘﾒﾝﾄ
	}
	nmisave( &suica_rcv_que.readpt, w_counter, sizeof(w_counter));					// ｵﾌｾｯﾄを停電保障で更新
}
