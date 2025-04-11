// MH810100(S) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)

//--------------------------------------------------
//		INCLUDE
//--------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "common.h"
#include "message.h"
#include "mem_def.h"
#include "prm_tbl.h"
#include "pktctrl.h"
#include "pkt_buffer.h"
// MH810100(S) 2020/06/19 車番チケットレス(#4199)
#include "ntnet_def.h"
// MH810100(E) 2020/06/19 車番チケットレス(#4199)


//--------------------------------------------------
//		定義
//--------------------------------------------------


//--------------------------------------------------
//		変数
//--------------------------------------------------
lcdbm_ctrl_t	lcdbm_ctrl;				///< LCDモジュール管理用ワーク

extern	ulong	Mov_cnt_dat[MOV_CNT_MAX];	// 動作ｶｳﾝﾄ

//--------------------------------------------------
//		以下、関数
//--------------------------------------------------
/**
 *	基本設定応答 受信時処理
 *
 *	@param[in]	p_rcv	受信データ格納用ワークへのポインタ
 *	@date	2019/11/25(月)
 */
void lcdbm_receive_config( lcdbm_rsp_config_t *p_rcv )
{
	uchar	ist;					// 割り込み処理からもCallされるため、割り込みステータス変更方式変更

	memset( &lcdbm_ctrl.receive_data.config, 0, sizeof(lcdbm_ctrl.receive_data.config));
	memcpy( lcdbm_ctrl.receive_data.config.prgm_ver,  p_rcv->prgm_ver,  sizeof(lcdbm_ctrl.receive_data.config.prgm_ver)  );
	memcpy( lcdbm_ctrl.receive_data.config.audio_ver, p_rcv->audio_ver, sizeof(lcdbm_ctrl.receive_data.config.audio_ver) );
	lcdbm_ctrl.receive_data.config.lcd_startup_time = p_rcv->lcd_startup_time;
	lcdbm_ctrl.receive_data.config.lcd_brightness   = p_rcv->lcd_brightness;

	ist = _di2();					// 割り込み禁止
	if( p_rcv->lcd_startup_time <= 999999999L ){
		Mov_cnt_dat[ LCD_LIGTH_CNT ] = p_rcv->lcd_startup_time;
	}else{
		Mov_cnt_dat[ LCD_LIGTH_CNT ] = 0L;
	}
	_ei2( ist );					// 割り込みステータスを元に戻す
}
// MH810100(S) 2020/06/19 車番チケットレス(#4199)

// センターが勝手に未使用に書き換えてくるので受信後に更新する（なぜか端末側で）
void Do_DiscountChecklcdbm_in_car_info(lcdbm_rsp_in_car_info_t *p_rcv)
{
	int cnt = 0;
	if(p_rcv){
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
		if ( p_rcv->crd_info.dtReqRslt.FeeCalcOnOff ) {			// 料金計算結果あり
			// 割引以外は削除して前に詰める
			while ( cnt < ONL_MAX_DISC_NUM ) {
// GG124100(S) R.Endo 2022/08/04 車番チケットレス3.0 #6125 【PK事業部要求】NT-NET精算データの割引情報に2001(買物金額合計)をセット
// 				if ( p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu > NTNET_SECTION_WARI_MAX ) {
				if (    (p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu > NTNET_SECTION_WARI_MAX)	// 割引情報以外
					 && (p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_KAIMONO_GOUKEI)		// 明細情報の買物金額合計以外
// GG129004(S) M.Fujikawa 2024/10/29 買物金額割引情報対応
					 && (p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_KAIMONO_INFO)		// 明細情報の買物割引情報以外
// GG129004(E) M.Fujikawa 2024/10/29 買物金額割引情報対応
				) {
// GG124100(E) R.Endo 2022/08/04 車番チケットレス3.0 #6125 【PK事業部要求】NT-NET精算データの割引情報に2001(買物金額合計)をセット
					// 最後尾以外は１つ前に上書きで移動
					if ( cnt < (ONL_MAX_DISC_NUM - 1) ) {
						memmove(&p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt],
							&p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[(cnt + 1)],
							sizeof(p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt]) * (ONL_MAX_DISC_NUM - 1 - cnt));
					}

					// 最後尾はクリア
					memset(&p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[(ONL_MAX_DISC_NUM - 1)],
						0, sizeof(p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt]));

					// １つ前に移動したのでいまのcntでやりなおす
					continue;
				}
				cnt++;
			}
		} else if ( p_rcv->crd_info.dtReqRslt.PayResultInfo ) {	// 割引情報あり
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)

// MH810100(S) 2020/08/04 仕様変更 #4573センターから受信した割引ステータスを変更しないようにする
// 		for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
// 			// 買物割引(金額=6) or 買物割引(時間=106)
// 			if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_AMT || 
// 				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_TIME ){
				
// 				if(p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].UsedDisc != 0){
// 					// 使用済み割引（金額/時間）が入っていたら使用済み＝2に強制的に変える
// 					p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscStatus = 2; 
// 				} 
// 			}
// // MH810100(S) 2020/07/17 仕様変更 #4553【連動不具合指摘事項】　買物割引と同様に、他のオンライン割引でも未割引（割引ステータス0）に対応してほしい
// 			// 002（店割引[金額]）または102（店割引[時間]）
// 			else if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_M || 
// 				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_T ){
// 				if(p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].UsedDisc != 0 && p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscInfo != 0){
// 					// 使用済み割引（金額/時間）が入っていたら 且つ 割引情報が000以外の場合 使用済み＝2に強制的に変える
// 					p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscStatus = 2; 
// 				}
// 			}
// // MH810100(E) 2020/07/17 仕様変更 #4553【連動不具合指摘事項】　買物割引と同様に、他のオンライン割引でも未割引（割引ステータス0）に対応してほしい
//		}
// MH810100(E) 2020/08/04 仕様変更 #4573センターから受信した割引ステータスを変更しないようにする
// MH810100(S) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)
		//・買物割引（6/106)・サービス券(1/101)・店割引(2/102)以外の場合は、RXM受信時に0クリアする。
		//・買物割引（6/106)・サービス券(1/101)・店割引(2/102)の場合は、RXM受信時に金額/時間を0クリアする
		for( cnt=0; cnt<ONL_MAX_DISC_NUM;  ){
			if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_AMT || 
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_TIME ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_M ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_T ||
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(入庫情報割引チェックの変更)
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_TKAK_M ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_TKAK_T ||
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(入庫情報割引チェックの変更)
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
// 50/150もここは通過させておく
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SYUBET_TIME ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SYUBET ||
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SVS_M ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SVS_T ){

				//・買物割引（6/106)・サービス券(1/101)・店割引(2/102)の場合は、RXM受信時に金額/時間を0クリアする
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].Discount = 0;
			}else{
				if( cnt < ONL_MAX_DISC_NUM ){	//
					if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu){ 
						if( cnt < (ONL_MAX_DISC_NUM -1 ) ){	//最後尾の場合はmemmoveしない
							// 前に１つ上書きで移動
							memmove(&p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt],&p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt+1],sizeof(stDiscount2_t)*(ONL_MAX_DISC_NUM-cnt-1));
						}
						// 最後尾をクリアしておく
						memset(&p_rcv->crd_info.dtCalcInfo.stDiscountInfo[ONL_MAX_DISC_NUM-1],0,sizeof(stDiscount2_t));
						// 1つ前に移動したのでいまのcntでやりなおす
						continue;
					}else{
						// 上詰めされているはずなので空になったら終了
						break;
					}
				}

			}
			cnt++;
		}
// MH810100(E) 2020/07/29 車番チケットレス(#4561 センターから受信した割引情報を変更する)

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
		}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
	}

}
// MH810100(E) 2020/06/19 車番チケットレス(#4199)

/**
 *	入庫情報 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@date	2019/11/25(月)
 */
void lcdbm_receive_in_car_info( lcdbm_rsp_in_car_info_t *p_rcv )
{
	memset( &lcdbm_rsp_in_car_info_recv, 0, sizeof(lcdbm_rsp_in_car_info_t));
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:541)対応
//	memcpy( &lcdbm_rsp_in_car_info_recv, &p_rcv->command.id, sizeof(lcdbm_rsp_in_car_info_t)  );
	memcpy( &lcdbm_rsp_in_car_info_recv, p_rcv, sizeof(lcdbm_rsp_in_car_info_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:541)対応
// MH810100(S) 2020/06/19 車番チケットレス(#4199)
	Do_DiscountChecklcdbm_in_car_info(&lcdbm_rsp_in_car_info_recv);
// MH810100(E) 2020/06/19 車番チケットレス(#4199)

	queset( OPETCBNO, LCD_IN_CAR_INFO, 0, NULL );
}

/**
 *	QRデータ 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@date	2019/11/27(水)
 */
void lcdbm_receive_QR_data( lcdbm_rsp_QR_data_t *p_rcv )
{
	memset( &lcdbm_rsp_QR_data_recv, 0, sizeof(lcdbm_rsp_QR_data_t));
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:542)対応
//	memcpy( &lcdbm_rsp_QR_data_recv, &p_rcv->command.id, sizeof(lcdbm_rsp_QR_data_t)  );
	memcpy( &lcdbm_rsp_QR_data_recv, p_rcv, sizeof(lcdbm_rsp_QR_data_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:542)対応
	queset( OPETCBNO, LCD_QR_DATA, 0, NULL );
}

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
/**
 *	レーンモニタデータ応答 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@param[out]	id		IDへのポインタ
 *	@param[out]	result	結果へのポインタ
 *	@date	2023/01/19(木)
 */
void lcdbm_receive_DC_LANE_res( lcdbm_rsp_LANE_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

/**
 *	精算応答データ 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@param[out]	id		IDへのポインタ
 *	@param[out]	result	結果へのポインタ
 *	@date	2019/11/27(水)
 */
void lcdbm_receive_RTPay_res( lcdbm_rsp_pay_data_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
/**
 *	領収証データ応答 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@param[out]	id		IDへのポインタ
 *	@param[out]	result	結果へのポインタ
 *	@date	2024/10/25
 */
void lcdbm_receive_RTReceipt_res( lcdbm_rsp_receipt_data_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

/**
 *	QR確定・取消応答データ 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@param[out]	id		IDへのポインタ
 *	@param[out]	result	結果へのポインタ
 *	@date	2019/11/27(水)
 */
void lcdbm_receive_DC_QR_res( lcdbm_rsp_QR_conf_can_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}

/**
 *	QRリーダ制御応答 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@date	2019/11/25(月)
 */
void lcdbm_receive_QR_ctrl_res( lcdbm_rsp_QR_ctrl_res_t *p_rcv )
{
	memset( &LcdRecv.lcdbm_rsp_QR_ctrl_res, 0, sizeof(lcdbm_rsp_QR_ctrl_res_t) );
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:543)対応
//	memcpy( &LcdRecv.lcdbm_rsp_QR_ctrl_res, &p_rcv->command.id, sizeof(lcdbm_rsp_QR_ctrl_res_t)  );
	memcpy( &LcdRecv.lcdbm_rsp_QR_ctrl_res, p_rcv, sizeof(lcdbm_rsp_QR_ctrl_res_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:543)対応
	queset( OPETCBNO, LCD_MNT_QR_CTRL_RESP, 0, NULL );
}

/**
 *	QR読取結果 受信時処理
 *
 *	@param[in]	p_rcv	受信データバッファへのポインタ
 *	@date	2019/11/25(月)
 */
void lcdbm_receive_QR_rd_rslt( lcdbm_rsp_QR_rd_rslt_t *p_rcv )
{
	memset( &LcdRecv.lcdbm_rsp_QR_rd_rslt, 0, sizeof(lcdbm_rsp_QR_rd_rslt_t));
// MH810100(S) S.Nishimoto 2020/04/08 静的解析(20200407:544)対応
//	memcpy( &LcdRecv.lcdbm_rsp_QR_rd_rslt, &p_rcv->command.id, sizeof(lcdbm_rsp_QR_rd_rslt_t)  );
	memcpy( &LcdRecv.lcdbm_rsp_QR_rd_rslt, p_rcv, sizeof(lcdbm_rsp_QR_rd_rslt_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 静的解析(20200407:544)対応
	queset( OPETCBNO, LCD_MNT_QR_READ_RESULT, 0, NULL );
}

/**
 *	LCD制御モジュールソフトウェアのバージョン情報を返す
 *
 *	@param[out]	dst		バージョン情報を格納するための配列へのポインタ
 *	@param[in]	size	最大サイズ
 *	@return				バージョンデータ長（バイト数）
 *	@date	2019/11/14
 *	@note
 *	-	LCDモジュールからの基本設定応答で取得済みのデータを返す。
 *	-	引数で示される配列に格納するバージョンデータには終端文字は含まない。
 *	-	返値で示されるバージョンデータ長には終端文字は含まない。
 */
uchar lcdbm_get_config_prgm_ver(char *dst, uchar size)
{
	uchar	*src = &lcdbm_ctrl.receive_data.config.prgm_ver[0];
	uchar	loop_max;
	uchar	count;

	loop_max = sizeof(lcdbm_ctrl.receive_data.config.prgm_ver);
	if (loop_max > size ) {
		loop_max = size;
	}
	for ( count = 0; count < loop_max; count++ ) {
		if ( *src == '\0' ) {
			break;
		}
		*dst++ = *src++;
	}
	return count;
}

/**
 *	音声データのバージョン情報を返す
 *
 *	@param[out]	dst		バージョン情報を格納するための配列へのポインタ
 *	@param[in]	size	最大サイズ
 *	@return				バージョンデータ長（バイト数）
 *	@date	2019/11/14
 *	@note
 *	-	LCDモジュールからの基本設定応答で取得済みのデータを返す。
 *	-	引数で示される配列に格納するバージョンデータには終端文字は含まない。
 *	-	返値で示されるバージョンデータ長には終端文字は含まない。
 */
uchar lcdbm_get_config_audio_ver(char *dst, uchar size)
{
	uchar	*src = &lcdbm_ctrl.receive_data.config.audio_ver[0];
	uchar	loop_max;
	uchar	count;

	loop_max = sizeof(lcdbm_ctrl.receive_data.config.audio_ver);
	if (loop_max > size ) {
		loop_max = size;
	}
	for ( count = 0; count < loop_max; count++ ) {
		if ( *src == '\0' ) {
			break;
		}
		*dst++ = *src++;
	}
	return count;
}

/**
 *	LCDモジュール起動時間を返す
 *
 *	@param[in]	none
 *	@param[out]	none
 *	@return				LCDモジュール起動時間
 *	@date	2019/11/14
 *	@note
 *	-	LCDモジュールからの基本設定応答で取得済みのデータを返す。
 */
ulong lcdbm_get_config_lcd_startup_time(void)
{
	return(lcdbm_ctrl.receive_data.config.lcd_startup_time);
}

/**
 *	LCD輝度を返す
 *
 *	@param[in]	none
 *	@param[out]	none
 *	@return				LCD輝度
 *	@date	2019/11/14
 *	@note
 *	-	LCDモジュールからの基本設定応答で取得済みのデータを返す。
 */
ushort lcdbm_get_config_lcd_brightness(void)
{
	return(lcdbm_ctrl.receive_data.config.lcd_brightness);
}

/**
 *	取得した基本設定応答の先頭ポインタを返す
 *
 *	@param[in]	none
 *	@param[out]	none
 *	@return				基本設定応答の先頭ポインタ
 *	@date	2019/11/14
 *	@note
 *	-	LCDモジュールからの基本設定応答で取得済みのデータを返す。
 */
uchar* lcdbm_get_config_ptr(void)
{
	return((uchar*)&lcdbm_ctrl.receive_data.config);
}

/**
 *	受信データを解析する
 *
 *	@param[in,out]	p			受信データ格納用ワークへのポインタ
 *	@param[in,out]	data_length	受信データ長
 *	@return	受信データのＩＤ（コマンドＩＤとサブコマンドＩＤを合算した４バイト値）
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/12/08(火)
 *	@note
 *	-	パケット通信タスクが管理するバッファから受信データを取り出す
 */
unsigned long lcdbm_analyze_packet_event( lcdbm_rsp_work_t *p, unsigned short *data_length )
{
	unsigned long	id;

	*data_length = PKTbuf_ReadRecvCommand( (unsigned char *)p );
	id = lcdbm_command_id( p->command.id, p->command.subid );

	return id;
}

//[]----------------------------------------------------------------------[]
///	@brief			コマンドメッセージチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: コマンド(コマンド＆サブコマンド)
///	@return			opeタスクに渡すメッセージ番号
///	@attention		そのままopeタスクに渡せないものは0を返す。
///	@note			コマンドに対応するメッセージ番号を返す。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/13<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort	lcdbm_check_message(const ulong data)
{
	ushort	msg;

	switch (data) {		// コマンド(2byte) + サブコマンド(2byte)
		case LCDBM_RSP_NOTIFY_CONFIG:		// 基本設定応答(ptkタスク内で情報保存)
		case LCDBM_RSP_TENKEY_KEYDOWN:		// テンキー押下情報(既存メッセージ番号に変換)
			msg = 0;
			break;
		case LCDBM_RSP_ERROR:				// エラー通知
			msg = LCD_ERROR_NOTICE;
			break;
		case LCDBM_RSP_NOTICE_OPE:			// 操作通知
			msg = 0;
			break;
		case LCDBM_RSP_IN_INFO:				// 入庫情報
			msg = 0;
			break;
		case LCDBM_RSP_QR_CONF_CAN_RES:		// QR確定・取消データ応答(ptkタスク内で処理)
			msg = 0;
			break;
		case LCDBM_RSP_QR_DATA:				// QRデータ
			msg = 0;
			break;
		case LCDBM_RSP_PAY_DATA_RES:		// 精算応答データ(ptkタスク内で処理)
			msg = 0;
			break;
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		case LCDBM_RSP_RECEIPT_DATA_RES:	// 領収証データ応答
			msg = 0;
			break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		case LCDBM_RSP_LANE_RES:			// レーンモニタ応答
			msg = 0;
			break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		case LCDBM_RSP_MNT_QR_CTRL_RES:		// QRリーダ制御応答
			msg = 0;
			break;
		case LCDBM_RSP_MNT_QR_RD_RSLT:		// QR読取結果
			msg = 0;
			break;
		case LCDBM_RSP_MNT_RT_CON_RSLT:		// リアルタイム通信疎通結果
			msg = LCD_MNT_REALTIME_RESULT;
			break;
		case LCDBM_RSP_MNT_DC_CON_RSLT:		// DC-NET通信疎通結果
			msg = LCD_MNT_DCNET_RESULT;
			break;
		default:							// 不明コマンド受信
			msg = 0;
			break;
	}

	return(msg);
}

//[]----------------------------------------------------------------------[]
///	@brief			キープアライブリトライ状態チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		cnt	:キープアライブリトライ回数(=設定№48-0020)
///	@return			TRUE:リトライOK/FALSE:リトライオーバ
///	@attention
///	@note
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/13<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
BOOL	lcdbm_check_keepalive_status(uchar cnt)
{
	if (lcdbm_ctrl.status.keepalive++ > cnt) {
		// キープアライブリトライオーバ
		return(FALSE);
	}

	return(TRUE);
}

//[]----------------------------------------------------------------------[]
///	@brief			キープアライブリトライカウンタリセット
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
///	@attention
///	@note
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/13<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_reset_keepalive_status(void)
{
	lcdbm_ctrl.status.keepalive = 0;
}

//@debug <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//--------------------------------------------------
//		テストコード
//--------------------------------------------------
#ifdef	CRW_DEBUG

void test_lcdbm_function( void )
{
//-#if 0
//-//
//-//	lcdbm_reset_audio_No();				// [0]=0,[1]=0,[2]=0,[3]=0,[4]=0
//-//
//-//	// 追加テスト						// 左記の関数実行後のlcdbm_ctrl.audio.issued_buf[]の値
//-//	lcdbm_push_audio_No( 1 );			// [0]=1
//-//	lcdbm_push_audio_No( 10 );			// [0]=1,[1]=10
//-//	lcdbm_push_audio_No( 100 );			// [0]=1,[1]=10,[2]=100
//-//	lcdbm_push_audio_No( 1000 );		// [0]=1,[1]=10,[2]=100,[3]=1000
//-//	lcdbm_push_audio_No( 10000 );		// [0]=1,[1]=10,[2]=100,[3]=1000,[4]=10000
//-//	lcdbm_push_audio_No( 65535 );		// [0]=10,[1]=100,[2]=1000,[3]=10000,[4]=65535	（最古のデータを追い出す）
//-//
//-//	// 削除テスト
//-//	lcdbm_pop_audio_No( 10 );			// [0]=100,[1]=1000,[2]=10000,[3]=65535,[4]=0
//-//	lcdbm_pop_audio_No( 20 );			// [0]=100,[1]=1000,[2]=10000,[3]=65535,[4]=0	（該当データがない場合は何もしない）
//-//	lcdbm_pop_audio_No( 1000 );			// [0]=100,[1]=10000,[2]=65535,[3]=0,[4]=0
//-//	lcdbm_pop_audio_No( 20000 );		// [0]=100,[1]=10000,[2]=65535,[3]=0,[4]=0
//-//	lcdbm_pop_audio_No( 65535 );		// [0]=100,[1]=10000,[2]=0,[3]=0,[4]=0
//-//
//-//	// 最古データを消すテスト
//-//	lcdbm_delete_oldest_audio_No();		// [0]=100,[1]=10000,[2]=0,[3]=0,[4]=0		（バッファが満杯でない場合は何もしない）
//-//	lcdbm_push_audio_No( 1 );			// [0]=100,[1]=10000,[2]=1,[3]=0,[4]=0
//-//	lcdbm_delete_oldest_audio_No();		// [0]=100,[1]=10000,[2]=1,[3]=0,[4]=0		（バッファが満杯でない場合は何もしない）
//-//	lcdbm_push_audio_No( 10 );			// [0]=100,[1]=10000,[2]=1,[3]=10,[4]=0
//-//	lcdbm_delete_oldest_audio_No();		// [0]=100,[1]=10000,[2]=1,[3]=10,[4]=0		（バッファが満杯でない場合は何もしない）
//-//	lcdbm_push_audio_No( 2000 );		// [0]=100,[1]=10000,[2]=1,[3]=10,[4]=2000
//-//	lcdbm_delete_oldest_audio_No();		// [0]=10000,[1]=1,[2]=10,[3]=2000,[4]=0	（最古のデータを消す）
//-//
//-//	// 全部クリアするテスト
//-//	lcdbm_push_audio_No( 65535 );		// [0]=10000,[1]=1,[2]=10,[3]=2000,[4]=65535
//-//	lcdbm_reset_audio_No();				// [0]=0,[1]=0,[2]=0,[3]=0,[4]=0
//-//	lcdbm_delete_oldest_audio_No();		// [0]=0,[1]=0,[2]=0,[3]=0,[4]=0	（バッファが満杯でない場合は何もしない）
//-//
//-#endif
}

#endif//CRW_DEBUG
//@debug >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// MH810100(E) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
