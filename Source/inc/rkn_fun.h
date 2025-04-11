/*[]----------------------------------------------------------------------[]*/
/*| 関数型宣言                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-02-01                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _RKN_FUN_H_
#define _RKN_FUN_H_

#include	"mem_def.h"
#include	"rkn_cal.h"

																		/*　　　　　　　　　　　　　　　　　*/
extern void		mc10(void);												/*　設定項目一括参照　　　　　　　　*/
extern void		mc101(void);											/*　設定項目一括参照（昼夜帯方式）　*/
extern void		mc102(void);											/*　設定項目一括参照（逓減帯方式）　*/
																		/*　　　　　　　　　　　　　　　　　*/
extern char		cm13(char,short *);										/*　月日時分範囲チェック　　　　　　*/
extern char		cm14(short);											/*　閏年チェック　　　　　　　　　　*/
extern void		cm27(void);												/*　フラグクリア　　　　　　　　　　*/
extern short	cm33(char);												/*　日付け変換　　　　　　　　　　　*/
extern short	cm41(short);											/*　設定項目参照　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　*/
extern void		et02(void);												/*　料金計算処理　　　　　　　　　　*/
extern void		et20(short);											/*　駐車料金額算出　　　　　　　　　*/
extern void		et2100(short);											/*　昼夜帯料金計算　　　　　　　　　*/
extern void		et2200(short);											/*　逓減帯料金計算　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　*/
extern void		et40(void);												/*　駐車料金計算処理　　　　　　　　*/
extern void		et42(void);												/*　時間割引処理　　　　　　　　　　*/
extern long		et421(void);											/*　通常時間割引額算出　　　　　　　*/
extern long		et422(void);											/*　定期時間割引額算出　　　　　　　*/
extern void		et43(void);												/*　料金割引処理　　　　　　　　　　*/
extern void		et44(void);												/*　回数券処理　　　　　　　　　　　*/
extern void		et45(void);												/*　サービス券処理　　　　　　　　　*/
extern void		et46(void);												/*　プリペイドカード処理　　　　　　*/
extern void		et47(void);												/*　定期券処理　　　　　　　　　　　*/
extern void		et471(short,short);										/*　時間帯定期券処理　　　　　　　　*/
extern char		et4711(short,struct CAR_TIM *);							/*　初回更新入庫時刻算出　　　　　　*/
extern void		et4712(short);											/*　駐車料金算出（定期帯）　　　　　*/
extern void		et472(void);											/*　割引時間定期券処理　　　　　　　*/
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
extern void		et48(void);												/*　紛失券処理　　　　　　　　　　　　　　　　*/
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
extern void		et51(void);												/*　店番号別割引処理　　　　　　　　*/
extern void		et57(void);												/*　車種切り替え処理　　　　　　　　*/
extern void		et59(void);												/*　料金差引割引　　　　　　　　　　*/
extern void		et90(void);												/*  クレジットカード */
extern void		et94(void);												/*　電子マネー処理		　　　　　　*/
extern void		et95(void);
extern void		et96(void);
extern void		et97(void);
																		/*　　　　　　　　　　　　　　　　　*/
extern void		et40_remote(void);										/*　リモート処理		　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　*/
extern char		ec081( char *, char * );								/*　有効開始終了範囲チェック　　　　*/
extern char		ec09(void);												/*　料金計算要求作成（ＶＬ）　　　　*/
extern char		ec191(char,char,char,char);								/*　車種切替料金計算要求作成　　　　*/
																		/*　　　　　　　　　　　　　　　　　*/
extern char		ec61(struct CAR_TIM *);									/*　料金体系算出処理　　　　　　　　*/
extern void		ec62(struct CAR_TIM *);									/*　日付１日繰下処理　　　　　　　　*/
extern void		ec63(struct CAR_TIM *);									/*　日付１日繰上処理　　　　　　　　*/
extern short	ec64(struct CAR_TIM *,struct CAR_TIM *);				/*　年月日時分比較処理　　　　　　　*/
extern void		ec65(short,short);										/*　日毎最大料金処理　　　　　　　　*/
extern void		ec65_1(char);											/*　日毎最大料金処理　　　　　　　　*/
extern short	hour612chk(short);										/*									*/
extern void		ec66(struct CAR_TIM *,long);							// 日付更新処理（減算） ※日付拡大※
extern char		ec67(short,struct CAR_TIM *,struct CAR_TIM *,char);		// 体系定期帯算出       ※引数追加※
extern unsigned long	ec71a(struct CAR_TIM *,struct CAR_TIM *);		// 時間差算出処理(ﾉｰﾏﾗｲｽﾞ)
extern	short	ta_st_sel( char, char );								// 定期曜日切換(0:00対応)
																		// 
extern long		ec68(long,short);										/*　消費税切上げ切捨て処理　　　　　*/
extern char		ec69(struct CAR_TIM *,short);							/*　定期券有効無効チェック　　　　　*/
extern void		ec70(struct CAR_TIM *,long);							/*　日付更新処理（加算）　　　　　　*/
extern unsigned long	ec71(struct CAR_TIM *,struct CAR_TIM *);		/*　時間差算出処理　　　　　　　　　*/
extern short 	ec72(struct CAR_TIM *,struct CAR_TIM *,struct CAR_TIM *);	/*　時間帯チエック　　　　　　　*/
extern char		ec82(short,struct CAR_TIM *,struct CAR_TIM *);			/*　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　*/
extern void		ryo_cal( char, unsigned short );						/*　　　　　　　　　　　　　　　　　*/
extern void		sryo_cal( ushort );										/*　　　　　　　　　　　　　　　　　*/
extern char		SvsTimChk( ushort );									/*　サービスタイム中チェック　　　　*/
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
extern int		vl_funchg( char f_Button, m_gtticstp *mag );
extern void		vl_tikchg( void );
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern void		vl_lcd_tikchg( void );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))

extern void		vl_carchg( ushort, uchar );								/*　　　　　　　　　　　　　　　　　*/
extern short	vl_paschg( void );									/*　　　　　　　　　　　　　　　　　*/
extern short	vl_mifpaschg( void );									/*　　　　　　　　　　　　　　　　　*/
extern short	vl_svschk( m_gtservic * );								/*　　　　　　　　　　　　　　　　　*/
// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) G.So ICクレジット対応
//extern	short	vl_sousai( ushort );
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
extern short	vl_prechg( m_gtprepid * );								/*　　　　　　　　　　　　　　　　　*/
extern	short	vl_kaschg( m_gtservic *mag );
extern void		vl_scarchg( ushort );									/*　　　　　　　　　　　　　　　　　*/
extern void		vl_passet( ushort );									/*　　　　　　　　　　　　　　　　　*/

extern void vl_cyushiset( ulong );

extern	int	is_paid_remote(Receipt_data* dat);
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
extern	int	is_ParkingWebFurikae( Receipt_data* dat );
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応
extern	short	is_paid_syusei(Receipt_data* dat);
extern	void	ryo_SyuseiRyobufSet( void );

extern	char	GET_RYO_SETMODE(uchar syubetu, uchar taikei);		/* ０時分割判定処理 */
extern	void	Change_Ryokin_Data( void );
extern	char	GET_PARSECT_NO(char no);
extern	char Get_Pram_Syubet( char no );
extern	long Carkind_Param( char no, char syu, char len, char pos);
// MH810100(S) Y.Watanabe 2020/02/26 車番チケットレス(買物割引対応)
extern	void ClearRyoCalDiscInfo( void );
// MH810100(E) Y.Watanabe 2020/02/26 車番チケットレス(買物割引対応)
// MH810100(S) 2020/06/10 #4216【連動評価指摘事項】車種設定種別時間割引設定時にQR割引(時間割引)を行っても適用されない
extern	void IncCatintime( struct	CAR_TIM		*wok_tm );
// MH810100(E) 2020/06/10 #4216【連動評価指摘事項】車種設定種別時間割引設定時にQR割引(時間割引)を行っても適用されない

#endif	// _RKN_FUN_H_
