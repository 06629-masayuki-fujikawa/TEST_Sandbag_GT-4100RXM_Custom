/*************************************************************************/ /**
 *	\file	SodiacAPI.h
 *	\brief	Sodiac ミドルウェアヘッダ Ver.2.30
 *	\author	Copyright(C) Arex Co. 2006-2011 All right Reserved.
 *	\author	株式会社アレックス 2006-2011
 *	\date	2011/04/20 Ver.2.30
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#ifndef	_defSodiacAPI_H_
#define	_defSodiacAPI_H_

/*************************************************************************/ /**
 *	処理結果列挙型
 **/ /*************************************************************************/
typedef enum _D_SODIAC_E
{
	D_SODIAC_E_OK = 0,			/**< 正常終了 */
	D_SODIAC_E_DATA ,			/**< データ異常 */
	D_SODIAC_E_MEM ,			/**< メモリ異常 */
	D_SODIAC_E_PAR ,			/**< パラメータ異常 */
	D_SODIAC_E_STATE ,			/**< 状態異常 */
	D_SODIAC_E_NO_SUPPORT ,		/**< 未サポート異常 */
	D_SODIAC_E_OTHER			/**< その他異常 */
}	D_SODIAC_E;

/*************************************************************************/ /**
 *	通知列挙型
 **/ /*************************************************************************/
typedef enum _D_SODIAC_ID
{
	D_SODIAC_ID_STOP = 0,		/**< 音声データ再生終了通知 */
	D_SODIAC_ID_NEXT,			/**< 次フレーズ指定可能通知 */
	D_SODIAC_ID_ERROR,			/**< 異常通知 */
	D_SODIAC_ID_PWMON,			/**< PWM出力開始通知(再生可能通知) */
	D_SODIAC_ID_PWMOFF,			/**< PWM出力停止通知 */
	D_SODIAC_ID_MUTEOFF,		/**< MUTEオフ通知 */
	D_SODIAC_ID_MUTEON			/**< MUTEオン通知 */
}	D_SODIAC_ID;

/*************************************************************************/ /**
 *	Sodiac bPause 設定用列挙型
 **/ /*************************************************************************/
typedef enum _D_SODIAC_BOOL
{
	D_SODIAC_BOOL_TRUE  = 1,			/**< 真 */
	D_SODIAC_BOOL_FALSE = 0				/**< 偽 */
}	D_SODIAC_BOOL;

/*************************************************************************/ /**
 *	Sodiac Kind 設定用列挙型
 **/ /*************************************************************************/
typedef	enum _D_SODIAC_KIND {
	D_SODIAC_KIND_DATA_OUT		= 0 ,	/**< データ再生 */
	D_SODIAC_KIND_PHRASE_OUT	= 1 ,	/**< 組み合わせ再生 */
	D_SODIAC_KIND_SILENCE_OUT	= 2 ,	/**< 無音再生 */
	D_SODIAC_KIND_BEEP_OUT		= 3		/**< 効果音再生 */
}	D_SODIAC_KIND;

/*************************************************************************/ /**
 *	Sodiac Option 設定用列挙型
 **/ /*************************************************************************/
typedef	enum _D_SODIAC_OPTION {
	D_SODIAC_OPTION_NONE       = 0x00 ,	/**< オプション未設定 */
	D_SODIAC_OPTION_PITCH_CONV = 0x01 ,	/**< ピッチ変換 */
	D_SODIAC_OPTION_SPEED_CONV = 0x02	/**< スピード変換 */
}	D_SODIAC_OPTION;

/*************************************************************************/ /**
 *	Nullポインタ設定用マクロ
 **/ /*************************************************************************/
#define	D_SODIAC_NULL	(0x00)

/*************************************************************************/ /**
 *	sodiac_get_version_info のパラメータ型
 **/ /*************************************************************************/
typedef struct _st_sodiac_version_info
{
	char	sodiac_version[8];	/**< Sodiac のバージョン番号 */
	char	build_version[8];	/**< Build番号 */
}	st_sodiac_version_info;

/*************************************************************************/ /**
 *	再生用パラメータ型
 **/ /*************************************************************************/
typedef struct _st_sodiac_param
{
	unsigned short	kind;		/**< D_SODIAC_KIND から選択 */
	unsigned short	option;		/**< D_SODIAC_OPTION から OR選択 */
	unsigned short	num;		/**< 再生番号　※無音・効果音時はms */
	unsigned short	volume;		/**< 0(最大)～32(無音) */
	unsigned short	pitch;		/**< 100を基準として大きいときに高く小さいときに低く */
	unsigned short	speed;		/**< 100を基準として大きいときに速く小さいときに遅く */
	unsigned short	ch;			/**< 再生するチャンネル */
}	st_sodiac_param;

/*************************************************************************/ /**
 *	コールバックポインタ型
 **/ /*************************************************************************/
typedef void (* fp_sodiac_callback)(int iCh, D_SODIAC_ID id, D_SODIAC_E err );

extern const char g_sodiac_copyright[];

#ifdef __cpluspluse
extern "C" {
#endif
/*************************************************************************/ /**
 *	ミドルウェアの初期化とタイマー動作クロックの設定
 *
 *	\param[in]	ulTimerClock  : タイマーに供給されるのクロック周波数(Hz)
 *	\retval		D_SODIAC_E_OK : 正常に登録された
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_init(unsigned long ulTimerClock);

/*************************************************************************/ /**
 *	再生停止を割り込みに通知
 *
 *	\param[in]	usStartTime1 : PWMオンからアンプパワーオンまでの時間(mS)
 *	\param[in]	usStartTime2 : アンプパワーオンから発声までの時間(mS)
 *	\param[in]	usEndTime    : アンプパワーオフからPWM停止の時間(mS)
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_amp_ctrl(unsigned short usStartTime1, unsigned short usStartTime2, unsigned short usEndTime);

/*************************************************************************/ /**
 *	再生の実行
 *
 *	\param[in]	pParam        : 再生するフレーズを指定する
 *	\retval		D_SODIAC_E_OK : 正常に登録された
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_execute(st_sodiac_param *pParam);

/*************************************************************************/ /**
 *	再生速度の設定
 *
 *	\param[in]	usSpeed       : 再生速度 普通が100で大きいほど速い
 *	\retval		D_SODIAC_E_OK : 正常に登録された
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_speed_conv(unsigned short usCh, unsigned short usSpeed);

/*************************************************************************/ /**
 *	再生音程の設定
 *
 *	\param[in]	usTone        : 再生音程 普通が100で大きいほど高い
 *	\retval		D_SODIAC_E_OK : 正常に登録された
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_pitch_conv(unsigned short usCh, unsigned short usTone);

/*************************************************************************/ /**
 *	音量の設定
 *
 *	\param[in]	usVolume      : 音量 最大が0で、最低は(C_SODIAC_VOLUMENUM-1)の値
 *	\retval		D_SODIAC_E_OK : 正常に登録された
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_volume_set(unsigned short usCh, unsigned short usVolume);

/*************************************************************************/ /**
 *	再生停止を割り込みに通知
 *
 *	\retval		D_SODIAC_E_OK : 正常に登録された
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_stop(unsigned short usCh);

/*************************************************************************/ /**
 *	再生の一時停止/再開
 *
 *	\param[in]	bPause : D_SODIAC_BOOL_TRUEで一時停止
 *						 D_SODIAC_BOOL_FALSEで一時停止解除
 *	\retval		D_SODIAC_E_OK : 正常に動作した
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_pause(unsigned short usCh, D_SODIAC_BOOL bPause);

/*************************************************************************/ /**
 *	再生できるフレーズの数を返す
 *
 *	\return		再生できるフレーズの数
 **/ /*************************************************************************/
extern unsigned short sodiac_get_phrase_num(void);

/*************************************************************************/ /**
 *	Sodiac ミドルウェアのバージョンを取得する
 *
 *	\param[out]	pInfo : バージョン情報を格納する領域を示すポインタ
 *	\retval		D_SODIAC_E_OK  : 正常に格納された
 *	\retval		D_SODIAC_E_PAR : 無効なポインタ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_get_version_info(st_sodiac_version_info *pInfo);

/*************************************************************************/ /**
 *	イベントフック関数の登録
 *
 *	\param[in]	pEventHook    : イベントフック関数へのポインタ
 *	\retval		D_SODIAC_E_OK : 正常に登録された
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_regist_eventhook(fp_sodiac_callback pEventHook);

/*************************************************************************/ /**
 *	再生情報取得
 *
 *	\param[out]	pState : !0 : 再生中 / 0 : 停止中
 *	\retval		D_SODIAC_E_OK : 正常に実行した
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_get_state(D_SODIAC_BOOL *pState);

#endif	/* _defSodiacAPI_H_ */
#ifdef __cpluspluse
}
#endif
/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex Co. Ltd, 2006-2011		  *
 **/ /*************************************************************************/
