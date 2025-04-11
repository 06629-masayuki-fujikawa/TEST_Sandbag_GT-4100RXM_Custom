/************************************************************************************/
/*　関数名称　　　: 磁気ﾘｰﾀﾞへのﾃﾞｰﾀ送信制御　　　　　		　　　　　　　　 　		*/
/*　関数シンボル　: cr_snd()  　 　　: 　　　　　　　　　　　	　　　　　　: 　	*/
/*　コンパイラ　　: MS-C ver.6.0　	: 　　　　　　　　　　　　　	　　　　: 　	*/
/*　ターゲットCPU : V25   　　　　　: 　　　　　　　　　　　　　　　	　　: 　	*/
/*　作成者（日付）: 戸田			  　　　　　　　　　　　　　　　	　　: 　	*/
/*　update:2004.9.16 T.Nakayama		  　　　　　　　　　　　　　　　	　　: 　	*/
/************************************************************************************/
#include	<string.h>										/*						*/
#include	"system.h"										/*						*/
#include	"Message.h"										/* Message				*/
#include	"mdl_def.h"										/* ﾓｼﾞｭｰﾙ間通信機器		*/
#include	"mem_def.h"										/*						*/
#include	"pri_def.h"										/*						*/
#include	"rkn_def.h"										/* 全デファイン統括		*/
#include	"rkn_cal.h"										/*						*/
#include	"ope_def.h"										/*						*/
#include	"prm_tbl.h"										/*						*/
#include	"fbapi.h"
#include	"mdl_def.h"

unsigned char	MTS_tx[256] = {0};							/*						*/
															/*						*/
															/*						*/
int				MTS_tseq[MTS_MAX] = {0, 0};					/* ｼｰｹﾝｼｬﾙ№			*/
															/*						*/
void	cr_snd( void )										/* ﾘｰﾀﾞﾃﾞｰﾀ送信			*/
{															/*						*/
	char bsiz;												/*						*/
															/*						*/
		/*--------------------------------------------------------------------------*/
		/* 挿入口券ｶｰﾄﾞﾁｪｯｸｱﾘの場合、ｶｰﾄﾞが引き抜かれるまで送信しない				*/
		/*--------------------------------------------------------------------------*/
	if(( ( mts_req & MTS_BCRWT1 ) == MTS_BCRWT1 ) &&		/* 挿入口ｶｰﾄﾞﾁｪｯｸｱﾘ?(Y)	*/
	   ( ( RED_stat[2] & TICKET_MOUTH) == TICKET_MOUTH ) ){/* 挿入口券あり?(Y)	*/
															/*						*/
		return;												/*						*/
	}														/*						*/
															/*						*/
	mts_req = ( mts_req & ~MTS_BCRWT1 );					/* 要求ｸﾘｱ				*/
															/*						*/
															/*						*/
		/*--------------------------------------------------------------------------*/
		/* 送信要求チェック															*/
		/*--------------------------------------------------------------------------*/
	if(( mts_req & MTS_BCR1 ) == 0 )						/* ﾘｰﾀﾞ送信要求なし?	*/
	{														/*						*/
		return;												/*						*/
	}														/*						*/
															/*						*/
	mts_req = ( mts_req & ~MTS_BCR1 );						/* 送信要求ｸﾘｱ			*/
															/*						*/
	MTS_tx[0] = '2';										/* 転送先(ﾘｰﾀﾞ)			*/
	MTS_tx[1] = (char)( MTS_tseq[MTS_RED] | '0' );			/* ｼｰｹﾝｼｬﾙ№			*/
	MTS_tseq[MTS_RED] = (char)(( MTS_tseq[MTS_RED] == 9 ) ?	/*						*/
							1 : ( MTS_tseq[MTS_RED] + 1 ));	/*						*/
	MTS_tx[2] = 0x02;										/* STX					*/
	MTS_tx[3] = 0x01;										/* ﾌﾞﾛｯｸ№	(ID1)		*/
															/*						*/
	bsiz = MDLdata[MTS_RED].mdl_data.idc2;					/*						*/
															/*						*/
	MTS_tx[4] = bsiz;										/* ｻｲｽﾞ		(ID2)		*/
	memcpy( &MTS_tx[5],										/*						*/
			&MDLdata[MTS_RED].mdl_data.rdat[0],				/*						*/
			(size_t)bsiz );									/*						*/
	MTS_tx[5 + bsiz] = 0x03;								/* ETX					*/
	MTS_tx[6 + bsiz] =										/* 奇数ﾊﾟﾘﾃｨ			*/
		(unsigned char)(~( bcccal( (char*)&MTS_tx[3], (unsigned short)(bsiz+3) )));
															/*						*/
	FBcom_SetSendData(MTS_tx, (unsigned short)(bsiz + 7));
	
	// 送信完了通知
	queset( OPETCBNO, ARC_CR_SND_EVT, 0, NULL );
	return;													/*						*/
}															/*						*/
															/*						*/
															/*						*/
/*==================================================================================*/

/*[]------------------------------------------------------------------------------[]*/
/*|	音声放送用受信処理															   |*/
/*|	音声放送用IBJからARCNETデータを受信し、データ処理する						   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : avm_rcv()													   |*/
/*| PARAMETER	 : void															   |*/
/*| RETURN VALUE : none															   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| date	   : 2004.10.15 T.Nakayama											   |*/
/*| 		   : 																   |*/
/*[]------------------------------------------------------------------------------[]*/
void	avm_snd( void )										/* ﾘｰﾀﾞﾃﾞｰﾀ送信			*/
{															/*						*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
//char bsiz;												/*						*/
//	unsigned	char		ptr,exec;						/* writeポインタ作業用	*/
//	unsigned	char		count,msg_count;				/* メッセージカウント用 */
//	unsigned	short		num;							/* 						*/
//	unsigned	short		ch;								/* ch番号				*/
	unsigned	char		count;							// メッセージカウント用
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
	struct		red_rec*	mdl;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
//	unsigned 	short		volume,ptn;						/* 音量設定				*/
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
	D_SODIAC_E	err;
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
//	ulong 	StartTime;
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
	mdl	 = &MDLdata[MTS_AVM].mdl_data;
		/*--------------------------------------------------------------------------*/
		/* 送信要求チェック															*/
		/*--------------------------------------------------------------------------*/
															/*						*/
															/*						*/
	if(( mts_req & MTS_BAVM1 ) == 0 )						/* 音声放送送信要求なし?*/
	{														/*						*/
		return;												/*						*/
	}														/*						*/
															/*						*/
	mts_req = ( mts_req & ~MTS_BAVM1 );						/* 要求ｸﾘｱ				*/
	MDLdata[MTS_AVM].mdl_size = 0;							/*		これを残しておかないとコマンド要求がこない				*/

	/*	初期化時エラーありの場合は要求処理を行わない */
	if( SODIAC_ERR_NONE != Avm_Sodiac_Err_flg )
	{
		return;
	}
	/* ここから下、旧ARCネットに対する登録(M_A_DEFN)・初期化(M_A_INIT)・M_A_MSAG(未使用)コマンドは無視する。*/
	switch( mdl->rdat[0] )												/*	要求コマンド種別により分岐				　	*/
	{																	/*                                            	*/
		case 0x0C:														/*  音声停止要求の場合                        	*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
//			ch = mdl->rdat[1];			
//			if( !AVM_Sodiac_Ctrl[ch].play_cmp ){
//				memset( &AVM_Sodiac_Ctrl[ch].sd_req[0].req_prm,	0x00, 
//					sizeof(AVM_SODIAC_CTRL));								/* 要求バッファallクリア						*/
//				AVM_Sodiac_Ctrl[ch].play_cmp = 1;							//音声再生停止中フラグON（停止中）
//				AVM_Sodiac_Ctrl[ch].stop = 1;								// 停止フラグON
//				AVM_Sodiac_Ctrl[ch].write_ptr = 0;							// データクリア
//				AVM_Sodiac_Ctrl[ch].read_ptr = 0;
//				AVM_Sodiac_Ctrl[ch].message_num = 0;
//
//				err	= sodiac_stop( ch );									/* 再生停止関数コール							*/
//				if( D_SODIAC_E_OK != err )						
//				{
//					AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );			/* エラー登録									*/
//				}
//				if( (ch == 0) && 											// ch0指定
//					(!avm_alarm_flg) && 									// 警報発報中以外
//					(AVM_Sodiac_Ctrl[1].message_num != 0) &&				// ch1メッセージあり
//					(AVM_Sodiac_Ctrl[1].play_message_cmp == 0) ) {			// ch1再生中
//					err	= sodiac_stop( 1 );									/* ch1再生停止関数コール							*/
//					if( D_SODIAC_E_OK != err ) {
//						AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );		/* エラー登録									*/
//					}
//				}
//				if((LagChkExe( OPETCBNO, 22 ) != 0)||(LagChkExe( OPETCBNO, 23 ) != 0)){
//					// オペレーション側で音声のインターバル中(Sodiacは停止中)に停止要求を受けた場合Sodiacは応答を返さないため強制終了する
//					Lagcan( OPETCBNO, 22 );										// インターバルタイマ停止
//					Lagcan( OPETCBNO, 23 );
//				}else{
//					Lagcan( OPETCBNO, 22 );										// インターバルタイマ停止
//					Lagcan( OPETCBNO, 23 );
//					// キャンセルした音声のD_SODIAC_ID_PWMOFFを受信するまで待つ
//					StartTime = LifeTim2msGet();
//					// NOTE:プログラムダウンロード+rismデータの大量送信状態で、クレジットの連続精算を行うと、割り込みに高負荷がかかり、音声停止要求後
//					// 動作停止するまで時間がかかるケースがある。タイムアウトで強制的にAVM_Sodiac_SemFree()を実行し、実際に音声合成モジュールが動作
//					// 停止していない状態で、ログ書き込みによるFROMのアクセスが発生すると、音声合成モジュールライブラリのsodiac_dataread()で無限ループ
//					// に陥るため対策する。
//					// 計測した結果最大3.15sのためマージンを約2倍の6sとする
//					while( 0 == LifePastTim2msGet(StartTime, 3000) ) {			// 最大6s待機
//						taskchg( IDLETSKNO );
//						if( AVM_Sodiac_Ctrl[ch].stop == 0 ) {					// 停止しているか（D_SODIAC_ID_PWMOFF受信済み）
//							break;
//						}
//					}
//				}
//				if(AVM_Sodiac_Ctrl[ch].stop != 0) {
//					// 一定時間経過してもstop == 1とならない場合はインターバル中のキャンセルと考える
//					// その場合はコールバックでD_SODIAC_ID_PWMOFFを受信しないので、ここでSemFreeを行う
//					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;				// 音声停止フラグON
//					AVM_Sodiac_SemFree();
//					AVM_Sodiac_Ctrl[ch].stop = 0;
//				}
//			}
			// mdl->rdat[0] == 0x0Cはan_stop()でset		mdl->rdat[1] = 終了チャネル
			// アナウンス終了要求
			if( PKTcmd_audio_end(
								0,			// 放送終了チャネル(0固定)
								0)			// 中断方法(0固定)
				 == FALSE ){
				// error
			}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス終了要求
			break;
		case 0x0A:														/*  音量設定要求の場合                        	*/
			// 毎回再生要求時に指定するので、不要
			break;
		case 0x09:														/*	テストコマンド(M_A_TEST)					*/
			memset(&Avm_Sodiac_Info,0x00,sizeof(st_sodiac_version_info));
			err = sodiac_get_version_info( &Avm_Sodiac_Info );
			if( D_SODIAC_E_OK != err )
			{
				AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );		/* エラー登録									*/
			}
			else
			{
				for( count = 0; count < 8; count++ )
				{
					/* バージョン情報の終端を取得	*/
					if( 0x00 == Avm_Sodiac_Info.sodiac_version[count] )
					{
						Avm_Sodiac_Info_end = count;
						break;
					}
				}	
			}
			break;
		case 0x0D:														/*	音声再生要求の場合							*/
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス開始要求
//			bsiz 		= mdl->idc2;									/* 	旧ARCNETに設定されたデータサイズを取得する  */
//			ch	 		= mdl->rdat[ bsiz -2];
//			ptr	 		= AVM_Sodiac_Ctrl[ch].write_ptr;				/*  											*/
//			
//			/* 音声番号が０番もしくは99以上の場合は破棄する */
//			if( !bsiz || (!mdl->rdat[1] && !mdl->rdat[2]) ){
//				break;
//			}
//			msg_count = (unsigned char)( bsiz - 5 );					/*	メッセージ数を算出	(msg_count/2)			*/
//
//			if( mdl->rdat[1] + mdl->rdat[2] == AVM_BOO ){				/* 警報音										*/
//				volume = CPrmSS[S_SYS][60];
//			}
//			else{														/* 通常音										*/
//				ptn = get_timeptn();
//				if( 0xff == pre_volume[0])
//				{
//					volume = get_anavolume((uchar)ch, (uchar)ptn);		/* 設定から音量取得								*/
//				}
//				else
//				{
//					volume = pre_volume[0];								/*	テスト音量取得								*/
//					pre_volume[0] = 0xff;								/*	テスト音量クリア							*/ 
//				}
//			}
//			if(volume == 0) {
//				return;								// 音量0なら放送しない
//			}
//			else if(volume > 15) {
//				volume = 15;
//			}
//
//			--volume;
//			volume = 15 - volume;					// sodiacに対する音量
//
//			/* メッセージ繰り返し時のためにメッセージ番号を割り振る */
//			if( 255 > AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num )
//			{
//				AVM_Sodiac_Ctrl[ch].message_num++;						/* メッセージ番号割り振り    */
//			}
//			else
//			{
//				AVM_Sodiac_Ctrl[ch].message_num = 1;				/* メッセージ番号割り振り    */
//			}
//			exec = 0;
//			for( count = 0; count < msg_count ; count+=2 )
//			{
//				if( (!mdl->rdat[ count+1] && !mdl->rdat[ count+2]) ){
//					continue;
//				}
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].resend_count		= (0x0F & mdl->rdat[ bsiz -4]);		/* 繰り返し回数取得			 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait				= mdl->rdat[ bsiz -3];				/* インターバル設定 　		 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.ch			= mdl->rdat[ bsiz -2];				/* ch設定 					 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.kind 		= D_SODIAC_KIND_PHRASE_OUT;			/* データ（組み合わせ）再生（無音対策用）	 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.option 		= D_SODIAC_OPTION_NONE;				/* option未設定				 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.volume		= volume;							/* 音量						 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.pitch		= 100;								/* ピッチ（基準値固定）		 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.speed		= 100;								/* スピード（基準値固定）	 */
//				num													= mdl->rdat[ count+1];
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.num			= num<<8;							/* 音声要求番号設定(上位)　  */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.num			|= mdl->rdat[ count+2];				/* 音声要求番号設定(下位)　  */
//				
//
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num 		= AVM_Sodiac_Ctrl[ch].message_num;
//
//				/* writeポインタインクリメント */	
//				if( AVM_Sodiac_Ctrl[ch].write_ptr >= (AVM_REQ_BUFF_SIZE -1) )
//				{
//					AVM_Sodiac_Ctrl[ch].write_ptr = 0;
//					ptr	= 0;
//				}
//				else
//				{
//					AVM_Sodiac_Ctrl[ch].write_ptr++;
//					ptr++;
//				}
//				exec++;
//			}
//			
//			if( !exec ){
//				return;																				/*								*/
//			}
//			AVM_Sodiac_Ctrl[ch].retry_count							= AVM_REQ_RETRY_MAX;			/* リトライ回数（２回）		 	*/
//
//			/* 音声再生停止中であれば再生要求を行う　*/
//			if( 1 == AVM_Sodiac_Ctrl[ch].play_cmp)
//			{
//				AVM_Sodiac_Ctrl[ch].resend_count_now	= AVM_Sodiac_Ctrl[ch].sd_req[AVM_Sodiac_Ctrl[ch].read_ptr].resend_count;	/* 残り再送回数設定	*/
//				/* 音声再生要求処理			*/
//				AVM_Sodiac_Execute( ch );
//			}
			// mdl->rdat[0] == 0x0Dはan_boo2()でset = AVM_BOOとAVM_IC_NG2の時
			// アナウンス開始要求
			if( PKTcmd_audio_start( 0, 1, (ushort)(mdl->rdat[1] + mdl->rdat[2]) ) == FALSE ){
				// error
			}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_ドアノブ戻し忘れ防止チャイム_ブザー音_アナウンス開始要求
			break;
		default:
			break;
	}
	return;													/*						*/
}															/*						*/
															/*						*/

