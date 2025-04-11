// MH810100(S) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
//[]----------------------------------------------------------------------[]
///	@brief			パケット通信関連制御関数定義
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pktctrl.h
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#ifndef	__PKTCTRL_H__
#define	__PKTCTRL_H__

#include 	<stdlib.h>
#include	<string.h>
#include	"system.h"
#include	"message.h"
#include	"prm_tbl.h"
#include	"mem_def.h"
#include	"lcdbm_ctrl.h"
#include	"pkt_def.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//--------------------------------------------------
//		プロトタイプ宣言
//--------------------------------------------------
// ステータス要求
BOOL PKTcmd_request_config( void );
//BOOL PKTcmd_request_status( void );

// 機器設定要求
// MH810100(S) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)
//BOOL PKTcmd_clock( void );
BOOL PKTcmd_clock( uchar type );
// MH810100(E) Y.Yamauchi 2019/12/18 車番チケットレス(メンテナンス)
BOOL PKTcmd_brightness( ushort brightness );
//BOOL PKTcmd_backlight( const uchar control );
//BOOL PKTcmd_refresh_enable( const BOOL enable );
//BOOL PKTcmd_Contrast( ushort usContrast );
//BOOL PKTcmd_tcpip( uchar control );

// テキスト表示
// ・１コマンド内に複数コントロールコードを指定する
void PKTcmd_text_begin( void );
BOOL PKTcmd_text_end( void );
BOOL PKTcmd_text_font( const uchar type );
BOOL PKTcmd_text_color( const ushort color );
BOOL PKTcmd_text_blink( const uchar line, const uchar column, const uchar blink, const uchar interval );
BOOL PKTcmd_text_clear( const uchar line, const uchar column, const uchar range );
BOOL PKTcmd_text_menu( const uchar line, const uchar column, const uchar *src );
BOOL PKTcmd_text_direct( const uchar line, const uchar column, const uchar *src );
BOOL PKTcmd_text_backgroundcolor( const ushort color );
BOOL PKTcmd_text_grachr( const ushort color, const uchar blink, const uchar interval, const uchar line, const uchar column, const uchar *src );
// ・１コマンド内にコントロールコードを一つのみ指定する
BOOL PKTcmd_text_1_font( const uchar font_type );
BOOL PKTcmd_text_1_color( const ushort color );
BOOL PKTcmd_text_1_blink( const uchar line, const uchar column, const uchar blink, const uchar interval );
BOOL PKTcmd_text_1_clear( const uchar line, const uchar column, const uchar range );
BOOL PKTcmd_text_1_menu( const uchar line, const uchar column, const uchar *src );
BOOL PKTcmd_text_1_direct( const uchar line, const uchar column, const uchar *src );
// MH810100(S) S.Takahashi 2019/12/25
BOOL PKTcmd_text_1_backgroundcolor( ushort color );
// MH810100(E) S.Takahashi 2019/12/25
BOOL PKTcmd_text_1_grachr( const ushort color, const uchar blink, const uchar interval, const uchar line, const uchar column, const uchar *src, ushort step, ushort totalstep );

// アナウンス要求
BOOL PKTcmd_audio_volume( uchar volume );
BOOL PKTcmd_audio_start( uchar startType, uchar reqKind, ushort no );
BOOL PKTcmd_audio_end( uchar channel, uchar intrpt_method );

// ブザー要求
BOOL PKTcmd_beep_volume(uchar volume);
BOOL PKTcmd_beep_start(uchar beep);
BOOL PKTcmd_alarm_start(uchar kind, uchar type);

// 精算機状態通知操作
BOOL PKTcmd_notice_ope( uchar ope_code, ushort status );
BOOL PKTcmd_notice_opn( uchar opn_cls, uchar reason );
BOOL PKTcmd_notice_pay( ulong event );
// MH810100(S) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
//BOOL PKTcmd_notice_dsp( uchar text_code, uchar status );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// BOOL PKTcmd_notice_dsp( uchar text_code, uchar status, ulong add_info );
BOOL PKTcmd_notice_dsp( uchar text_code, uchar status, ulong add_info, uchar *str, ulong str_size );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// MH810100(E) S.Fujii 2020/07/20 車番チケットレス(電子決済端末対応)
BOOL PKTcmd_notice_alm( ushort event );
// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
BOOL PKTcmd_notice_del( uchar kind, uchar status );
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）

// 精算・割引情報通知
BOOL PKTcmd_pay_rem_chg( lcdbm_cmd_pay_rem_chg_t *src );
BOOL PKTcmd_QR_conf_can( lcdbm_cmd_QR_conf_can_t *src );
BOOL PKTcmd_QR_data_res( ulong id, uchar result );
// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
BOOL PKTcmd_remote_time_start( lcdbm_cmd_remote_time_start_t *src );
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
BOOL PKTcmd_lane_data( lcdbm_cmd_lane_t *src );
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

// メンテナンスデータ
BOOL PKTcmd_mnt_qr_ctrl_req( unsigned char ctrl_cd );
BOOL PKTcmd_mnt_rt_con_req( void );

// ----------------------------------------------- //
// 状態セット/取得関数
// ----------------------------------------------- //

// 起動完了状態
extern void PktStartUpStsChg( uchar faze );		// 起動完了状態更新

// リセット要求状態
extern void PktResetReqFlgSet( uchar rst_req );	// リセット要求フラグセット

// 復旧データ状態
extern uchar pkt_get_restore_unsent_flg( void );
extern void pkt_set_restore_unsent_flg( uchar flg );

//--------------------------------------------------
//		テストコード
//--------------------------------------------------
#ifdef	CRW_DEBUG
void test_packet_function( void );
#else
#define	test_packet_function()	;	// リリースビルド時に"No prototype function"ワーニングが出ないようにする
#endif//CRW_DEBUG

#ifdef __cplusplus
};
#endif // __cplusplus

#endif	// __PKTCTRL_H__
// MH810100(E) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
