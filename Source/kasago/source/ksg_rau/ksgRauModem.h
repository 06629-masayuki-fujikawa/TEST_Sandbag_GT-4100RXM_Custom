/************************************************************************************************/
/*																								*/
/*	ﾓｼﾞｭｰﾙ名称	:ksgRauModem.h		:---------------------------------------------------------: */
/*	ﾓｼﾞｭｰﾙｼﾝﾎﾞﾙ	:					: Modem 関連											  : */
/*																								*/
/************************************************************************************************/

/* ksgRauModemMain.c																					*/
extern	void			KSG_RauMdmRcvQueInit( void );						/* 受信ｷｭｰの初期化			*/
extern	void			KSG_RauMdmRcvQueSet( unsigned char *, unsigned int );
																			/* 受信ｷｭｰの書出			*/
extern	void			KSG_RauMdmSndQueInit( void );						/* 送信ｷｭｰの初期化			*/
extern	void			KSG_RauMdmSndQueSet( unsigned char *, unsigned int, unsigned char );
																			/* 送信ｷｭｰの書出			*/
//extern  void			KSG_RauMdmTimerInit( void );
extern	unsigned char	KSG_RauMdmSndQueRead( unsigned char *pData, unsigned int *len, unsigned char *ch, unsigned char kind );
extern	unsigned char	KSG_RauMdmRcvQueRead( unsigned char *pData, unsigned short *len );
																			/* 送信ｷｭｰの読出			*/
extern	void			KSG_RauMdm_SumCheck( unsigned char *, unsigned short , unsigned char * );
																			/* ｻﾑﾁｪｯｸ					*/
extern	void			KSG_RauMdmPostPppClose( void );
extern	void			KSG_RauClosePPPSession( void );
extern	unsigned char	KSG_RauGetMdmCutState( void );
extern	void			KSG_RauSetMdmCutState( unsigned char modemCutRequestState );

/* ksgRauModemControl.c																					*/
extern	void			KSG_RauMdmControl( void );							/* ﾓﾃﾞﾑ制御					*/
extern	void			KSG_RauMdmAntLevMode( void );						/* ｱﾝﾃﾅﾚﾍﾞﾙ、受信電力指数取得 */
extern	void			KSG_RauMdmResultCheck( void );						/* ﾘｻﾞﾙﾄｺｰﾄﾞ解析			*/
extern	void			KSG_RauAntenaLevelSet( void );						/* ｱﾝﾃﾅﾚﾍﾞﾙ取得				*/
extern	void			KSG_RauReceptionLevelSet( void );					/* 受信電力指標取得			*/
extern	void			KSG_RauModem_ON( void );							/* ﾓﾃﾞﾑ接続要求				*/
extern	void			KSG_RauModem_OFF( void );							/* ﾓﾃﾞﾑ切断要求				*/
extern	void			KSG_RauModem_OFF_2( void );							/* ﾓﾃﾞﾑ切断要求-2 ﾃﾞｰﾀ再送ﾀｲﾏｰ起動なし */
extern	void			KSG_RauMdmInitReq( void );							/* ﾓﾃﾞﾑ初期化要求			*/
extern	void			KSG_RauMdmInitCmdSetFOMA( signed short );			/* ﾓﾃﾞﾑ初期化ｺﾏﾝﾄﾞ送信		*/
extern	void			KSG_RauMdmAntLevelCmdSetFOMA( signed short );		/* ｱﾝﾃﾅﾚﾍﾞﾙ、受信電力指数取得ｺﾏﾝﾄﾞ送信 */
extern	void			KSG_RauMdmTxStart( void );							/* ﾓﾃﾞﾑｺﾏﾝﾄﾞ送信開始		*/
extern	void			KSG_RauMdmConnect( void );							/* ﾎｽﾄ接続要求				*/
extern	void			KSG_RauMdmSigInfoInit( void );						/* RS232C ﾁｬﾀ取りｴﾘｱ初期化	*/
extern	void			KSG_RauTResetOut( void );							/* 機器ﾘｾｯﾄ出力処理			*/
extern	void			KSG_RauTResetOutFOMA( void );						/* 機器ﾘｾｯﾄ出力処理			*/
extern	void			KSG_RauTResetOutFOMA_NTNET( void );					/* 機器ﾘｾｯﾄ出力処理			*/
extern	void			KSG_RauTResetOut_UM03( void );						/* 機器ﾘｾｯﾄ出力処理(UM03-KO)*/
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御
extern	void			KSG_RauTResetOut_UM04( void );						/* 機器ﾘｾｯﾄ出力処理(UM04-KO)*/
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)対応 モデム制御

/* ksgRauSci.c																							*/
extern	void			KSG_RauSciLineCheck( void );						/* ﾗｲﾝ状態確認				*/
extern	unsigned short	KSG_CD_ON( void );									/* 							*/
extern	unsigned short 	KSG_CS_ON( void );									/*							*/

/* ksgRauModemDriver.c																					*/
extern	void			KSG_RauMdmTimerInit( void );						/* ﾀｲﾏ初期化処理			*/
extern	void			KSG_RauMdm10msInt( void );							/* 10msﾀｲﾏ処理				*/
extern	void			KSG_RauMdm100msInt( void );							/* 100msﾀｲﾏ処理				*/

/* ksgRauSysError.c																						*/
extern	void			KSG_RauSysErrSet( unsigned char n , unsigned char s );


#define RAU_ERR_FMA_ANTENALOW1		31										/* 接続時のｱﾝﾃﾅﾚﾍﾞﾙが 1		*/
#define RAU_ERR_FMA_ANTENALOW2		32										/* 接続時のｱﾝﾃﾅﾚﾍﾞﾙが 2		*/
#define	RAU_ERR_FMA_NOCARRIER		34										/* NO CARRIRE検出			*/
#define	RAU_ERR_FMA_SIMERROR		35										/* SIMカード外れ検知		*/
#define	RAU_ERR_FMA_ADAPTER_NG		36										/* FOMAアダプタ設定不良		*/
#define	RAU_ERR_FMA_MODEMERR_DR		74										/* DR信号が 3秒経過してもONにならない(MODEM ON時) */
#define	RAU_ERR_FMA_MODEMERR_CD		75										/* CD信号が60秒経過してもOFFにならない(MODEM OFF時) */
#define	RAU_ERR_FMA_PPP_TIMEOUT 	76										/* PPPコネクションタイムアウト */
#define	RAU_ERR_FMA_RESTRICTION		77										/* RESTRICTION(規制中)を受信 */
#define RAU_ERR_FMA_MODEMPOWOFF		78										/* MODEM 電源OFF			*/
#define RAU_ERR_FMA_ANTENALOW		79										/* 接続時のｱﾝﾃﾅﾚﾍﾞﾙが 0		*/
#define	RAU_ERR_FMA_REDIALOV		94										/* リダイアル回数オーバー	*/
