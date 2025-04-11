#ifndef	_APPSERV_H_
#define	_APPSERV_H_
/*[]-----------------------------------------------------------------------[]*/
/*| Backup/Restore, Download/Upload common include                          |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005/06/24                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*----------------------------------------------------------*/
/*	パラメータデータ情報テーブル 							*/
/*----------------------------------------------------------*/

/* セクション毎の情報 */
typedef struct {
	void				*address;		/* セクション先頭アドレス */
	ushort				item_num;		/* セクション内の項目数 */
}t_AppServ_SectInfo;

/* パラメータ情報テーブル */
typedef struct {
	ushort				csection_num;	/* 共通パラメータ セクション数 */
	ushort				psection_num;	/* 個別パラメータセクション数 */
	t_AppServ_SectInfo	csection[64];	/* 共通パラメータ */
	t_AppServ_SectInfo	psection[64];	/* 個別パラメータ */
}t_AppServ_ParamInfoTbl;

// 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ条件変更
extern	ushort	AppServ_PParam_Copy;			// FLASH ⇒ RAM 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ有無
												//	OFF:個別ﾊﾟﾗﾒｰﾀをｺﾋﾟｰしない
												//	ON :個別ﾊﾟﾗﾒｰﾀをｺﾋﾟｰする

/*----------------------------------------------------------*/
/*	バックアップデータ情報テーブル 							*/
/*----------------------------------------------------------*/

/* エリア毎の情報 */
typedef struct {
	void				*address;		/* エリア先頭アドレス */
	ulong				size;			/* エリアサイズ */
}t_AppServ_AreaInfo;

typedef struct {
	ulong				from_address;	// バックアップ先FROMの先頭アドレス
}t_AppServ_FromInfo;

/* エリア情報テーブル */
typedef struct {
	ushort				area_num;	/* エリア数 */
	t_AppServ_AreaInfo	area[64];	/* 各エリアごとの情報 */
	t_AppServ_FromInfo	from_area[64];
} t_AppServ_AreaInfoTbl;


/*----------------------------------------------------------*/
/*	パラメータUpload/Download用 保存データ					*/
/*----------------------------------------------------------*/

typedef struct {			/* BIN→CSV変換処理の保存データ */
	ulong 					datasize;			/* セットバッファサイズ 						*/
												/* (初期化時に要求元から渡される)				*/
	uchar					Phase;				/* 変換処理状態 0=見出しセット中、1=データセット中 */
	uchar					Kind;				/* 対象パラメータ指定（0=共通，1=個別）			*/
	ushort					Section;			/* 次処理するセクション番号（0～）				*/
	ushort					Item;				/* 次処理するセクション内アイテム番号（0～）	*/
	ushort					CallCount;			/* Callされた回数								*/
} t_AppServ_Bin_toCsv_Param;

typedef struct {			/* CSV→BIN変換処理の保存データ */
	char					*param_image;		/* バイナリイメージ保存エリアへのポインタ 		*/
												/* (初期化時に要求元から渡される)				*/
	ulong					image_size;			/* param_imageエリアのサイズ 					*/
												/* (初期化時に要求元から渡される)				*/
	ushort					CallCount;			/* Callされた回数								*/
//	ulong					WriteIndex;			/* 次書込み位置(offset) 						*/
} t_AppServ_Csv_toBin_Param;

typedef struct {			/* BIN,CSV変換処理の保存データ */
	t_AppServ_ParamInfoTbl 	param_info;			/* パラメータ情報テーブルへのポインタ 			*/
												/* BIN→CSV，CSV→BINどとらも同じデータなので 	*/
												/* 後者優先で保存する							*/
	t_AppServ_Bin_toCsv_Param	Bin_toCsv_Param;
	t_AppServ_Csv_toBin_Param	Csv_toBin_Param;
} t_AppServ_BinCsvSave;

/*----------------------------------------------------------*/
/*	車室パラメータ書き込み用 保存データ						*/
/*----------------------------------------------------------*/

typedef struct {
	uchar	sts;
	char	*image;
}t_AppServ_LockInfoFukuden;

/*----------------------------------------------------------*/
/*	ＡＰＩ外部参照（プロトタイプ）							*/
/*----------------------------------------------------------*/
extern	void	AppServ_MakeBackupDataInfoTable( t_AppServ_AreaInfoTbl *pTable );
extern	void	AppServ_MakeParamInfoTable( t_AppServ_ParamInfoTbl *pTable );
extern	void	AppServ_FormatHeaderFooter( char formated[4][36], const char *text, ushort text_len );
// 加盟店ダウンロード時、AppServ_FormatSyomeiKamei()ではなくppServ_FormatHeaderFooterを使用している
// 処理内容がHeaderFooterと全く同じなので統合する
#define	AppServ_FormatSyomeiKamei		AppServ_FormatHeaderFooter
#define	AppServ_InsertCrlfToSyomeiKamei	AppServ_InsertCrlfToHeaderFooter

extern	ushort	AppServ_InsertCrlfToEdyAtCommand( char *inserted, const char text[6][20] );
extern	void	AppServ_FormatEdyAtCommand( char formated[6][20], const char *text, ushort text_len );

extern	ushort	AppServ_InsertCrlfToHeaderFooter( char *inserted, const char text[4][36] );
extern	uchar	AppServ_ConvParam2CSV_Init( const t_AppServ_ParamInfoTbl *param_info, ulong datasize );
extern	void	AppServ_CnvParam2CSV( char *csvdata, ushort *csvdata_len, uchar *finish );
extern	uchar	AppServ_ConvCSV2Param_Init( const t_AppServ_ParamInfoTbl *param_info, 
									char *param_image, ulong image_size );
extern	uchar	AppServ_CnvCSV2Param(const char *csvdata, ushort csvdata_len, ulong *finish );
extern	ulong	AppServ_ConvCsvParam_ImageAreaSizeGet( const t_AppServ_ParamInfoTbl *param_info );
extern	void	AppServ_FukudenProc( char RAMCLR, char f_CPrmDefaultSet );
extern	void	DataSumUpdate( ushort kind );
extern	uchar	DataSumCheck( ushort kind );
extern	void	AppServ_LogFlashWriteReq( short id );
extern	uchar	AppServ_IsLogFlashWriting( short id );

extern	uchar	AppServ_CnvCSV2LockInfo_Init( char *lock_info_image, ulong image_size );
extern	uchar	AppServ_CnvCSV2LockInfo(const char *csvdata, ushort csvdata_len, ulong *finish );
extern	uchar	AppServ_CnvLockInfo2CSV_Init( void );
extern	void	AppServ_CnvLockInfo2CSV( char *csvdata, ushort *csvdata_len, uchar *finish );
extern	void	AppServ_ConvErrArmCSV( char *csvdata, ulong *csvdata_len, uchar *finish );
extern	void	AppServ_ConvErrArmCSV_Init( uchar kind );
extern	uchar	AppServ_CnvCSVtoFLAPDT_Init( char *, ulong );
extern	uchar	AppServ_CnvCSVtoFLAPDT(const char *, ushort, ulong * );
extern	ushort	AppServ_InsertCrlfToTextData( char *inserted, const char text[][36], uchar line_num );
extern	void	AppServ_FormatTextData( char formated[][36], const char *text, ushort text_len, uchar line_num );
extern	void	AppServ_SaleTotal_LOG_edit( char *, ulong *, uchar * );
extern	void	AppServ_SaleTotal_LOG_Init( uchar, ulong );
extern	void	AppServ_ConvChkResultCSV( char *csvdata, ulong *csvdata_len, uchar *finish );
extern	void 	Get_Result_dat(uchar no, uchar cnt, uchar *dat);
extern	void	AppServ_ConvChkResultCSV_Init(void);
extern	void	AppServ_Restore_BackupData(const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_ToV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_V01toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_V02toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
// MH321800(S) Y.Tanizaki ICクレジット対応
extern	void	AppServ_Restore_SYUKEI(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_SYUKEISP(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810105 MH321800(S) 「未了残高照会完了」時にも未了確定をカウントする
extern	void	AppServ_Restore_ECALAMLOG(ushort area, t_AppServ_AreaInfoTbl *area_tbl_wk);
// MH810105 MH321800(E) 「未了残高照会完了」時にも未了確定をカウントする
// MH810100(S) K.Onodera 2019/11/29 車番チケットレス(メンテナンス)
extern	ushort	AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( char SecKind, ushort SecNum );
// MH810100(E) K.Onodera 2019/11/29 車番チケットレス(メンテナンス)

#endif
