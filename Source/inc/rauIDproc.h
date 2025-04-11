#ifndef	RAUIDPROC_H
#define	RAUIDPROC_H
/****************************************************************************/
/*																			*/
/****************************************************************************/


/****************************************************************/
/*		データテーブル情報構造体								*/
/****************************************************************/

#define	REQUEST_TABLE_MAX	15			/* 通信要求データ・テーブルクリア要求データで要求される最大テーブル数 */


#if 1
typedef struct {
	uchar		*pc_AreaTop;			// 領域先頭アドレス
	ulong		ul_AreaSize;			// 領域サイズ
	ulong		ul_NearFullMaximum;		// ニアフルアラーム上限値
	ulong		ul_NearFullMinimum;		// ニアフルアラーム解除値
	ushort		i_NearFullStatus;		// ニアフル発生状態
	ushort		i_NearFullStatusBefore;	// ニアフル発生状態（前回）
	uchar		*pc_ReadPoint;			// データ読み込み位置
	//ulong		ul_ReadPoint;			// データ読み込み位置
	uchar		*pc_WritePoint;			// データ書き込み位置
	//ulong		ul_WritePoint;			// データ書き込み位置
	ushort		ui_DataCount;			// データ件数
	ushort		ui_SendDataCount;		// 送信対象データ件数
	char		c_DataFullProc;			// バッファフル時データ処理法
	uchar		uc_DataId;
	ushort		ui_syuType;				// 集計タイプ
} RAU_DATA_TABLE_INFO;
#endif

/*------------------------送信対象情報----------------------------------------*/
//--- バッファごとの送信情報 ---
typedef struct {
	uchar	send_req;			// 送信対象として指定されたかどうか
	ushort	send_data_count;	// 送信対象件数
	ushort	send_complete_count;// 送信完了件数
	ushort	fill_up_data_count;	// 充填完了データ数
	ushort	crc_err_data_count;	// CRCエラーだったデータ数
	RAU_DATA_TABLE_INFO *pbuff_info;
} T_SEND_DATA_ID_INFO;

//--- 送信情報のバッファ分の配列 ---
typedef struct {
	T_SEND_DATA_ID_INFO send_data_id_info[17];
} T_SEND_DATA_INFO;

//--- Dopa送信用NT-NETデータ情報 ---
typedef struct {
	uchar	*nt_data_len;		// NT-NETデータ長位置
	uchar	*nt_data_start;		// NT-NETデータ開始位置
	uchar	*nt_data_end;		// NT-NETデータ終了位置
	uchar	*nt_data_crc;		// NT-NET CRC  終了位置
	uchar	*nt_data_next;		// NT-NETデータ次回読み込み位置
} T_SEND_NT_DATA;


//--- T合計　金銭管理　NT-NETデータ情報 ---
/* NT buffer structure */
struct _ntbuf {
	char	*next;
	long	length;
};
#define	_GET_DATA(bf)	((char*)bf+sizeof(struct _ntbuf))

/* NT buffer management structure */
struct _ntman {
	char	*top;
	char	*bottom;
	char	*freearea;
	char	*sendtop;
	char	*sendnow;
};

#define	ALIGN(n)	((n+1)&0xfffffffe)


/****************************************************************/
/*		端末送信用キュー										*/
/****************************************************************/
#define	TERM_QUE_MAX	1

/****************************************************************/
/*		ＨＯＳＴ送信用キュー（端末からスルー)					*/
/****************************************************************/
#define	HOST_QUE_MAX	1

/****************************************************************/
/*		ＨＯＳＴ送信用キュー（テーブルデータ)					*/
/****************************************************************/
#define	HOST_TABLE_QUE_MAX	1

typedef struct _host_table_no {
	ushort		ui_rec_old_bk;							/* 最古レコード番号バックアップ			*/
	ushort		ui_rec_new_bk;							/* 最新レコード番号バックアップ			*/
	ushort		ui_rec_now_bk;							/* 現レコード数バックアップ				*/
	ushort		ui_rec_y_old_bk;						/* 未送信最古レコード番号バックアップ	*/
	ushort		ui_rec_y_now_bk;						/* 未送信レコード数バックアップ			*/
	ushort		ui_rec_no_bk;							/* キューのデータの最後のレコード№		*/
	ushort		ui_rec_cnt_bk;							/* キューのデータのレコード数			*/
	ulong		ul_rec_tbl_no_bk;						/* キューのデータのテーブルデータ№		*/
} HOST_TABLE_NO;

/****************************************************************/
/*		送信作成待ち電文IDキュー								*/
/****************************************************************/
#define	RAU_ID_QUE_MAX_T	1
#define	RAU_ID_QUE_MAX_H	1

/****************************************************************/
/*		ＲＡＵ設定電文構造体									*/
/****************************************************************/
typedef struct _RAU_CONFIG {
	char		proc_kind[2];			/* 処理区分 */
	char		parking_id[8];			/* 駐車場ＩＤ */
	char		password[8];			/* パスワード */
	char		host_tel1[14];			/* ホスト側ＴＥＬ番号１ */
	char		host_tel2[14];			/* ホスト側ＴＥＬ番号２ */
	char		host_tel3[14];			/* ホスト側ＴＥＬ番号３ */
	char		id22_23_tel_flg;		/* ＩＤ自動発信フラグ  （精算データ）*/
	char		id22_23_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （精算データ）*/
	char		id30_41_tel_flg;		/* ＩＤ自動発信フラグ  （集計データ）*/
	char		id30_41_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （集計データ）*/
	char		id121_tel_flg;			/* ＩＤ自動発信フラグ  （アラームデータ）*/
	char		id121_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （アラームデータ）*/
	char		id123_tel_flg;			/* ＩＤ自動発信フラグ  （操作モニターデータ）*/
	char		id123_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （操作モニターデータ）*/
	char		id120_tel_flg;			/* ＩＤ自動発信フラグ  （エラーデータ）*/
	char		id120_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （エラーデータ）*/
	char		id20_21_tel_flg;		/* ＩＤ自動発信フラグ  （入庫出庫データ）*/
	char		id20_21_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （入庫出庫データ）*/
	char		id122_tel_flg;			/* ＩＤ自動発信フラグ  （モニターデータ）*/
	char		id122_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （モニターデータ）*/
	char		id131_tel_flg;			/* ＩＤ自動発信フラグ  （コイン金庫集計データ）*/
	char		id131_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （コイン金庫集計データ）*/
	char		id133_tel_flg;			/* ＩＤ自動発信フラグ  （紙幣金庫集計データ）*/
	char		id133_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （紙幣金庫集計データ）*/
	char		id236_tel_flg;			/* ＩＤ自動発信フラグ  （駐車台数データ）*/
	char		id236_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （駐車台数データ）*/
	char		id237_tel_flg;			/* ＩＤ自動発信フラグ  （区画台数・満車データ）*/
	char		id237_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （区画台数・満車データ）*/
	char		id126_tel_flg;			/* ＩＤ自動発信フラグ  （金銭管理データ）*/
	char		id126_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （金銭管理データ）*/
	char		id135_tel_flg;			/* ＩＤ自動発信フラグ  （釣銭管理集計データ）*/
	char		id135_tel_no;			/* ＩＤ自動発信ＴＥＬ番号  （釣銭管理集計データ）*/
	char		yobi[8];				/* 予備 */
	char		tel_priority[3];		/* 発信先優先順位  */
	char		speed;					/* 通信速度	 */
	char		data_len;				/* データ長 */
	char		stop_bit;				/* ストップビット */
	char		parity_bit;				/* パリティビット */
	char		deta_full_proc;			/* データ処理方法 */
	char		retry[2];				/* リトライ回数 */
	char		time_out[2];			/* タイムアウト */
	char		modem_retry;			/* モデム接続リトライ */
	char		point_change;			/* 接点変化自動発信 */
	char		memory_alarm;			/* メモリアラーム発信 */
	char		rau_check_sum[4];		/* ＲＡＵチェックサム */
	char		rau_version[8];			/* ＲＡＵバージョン */
	char		init_data[64];			/* 初期化データ */
	uchar		IBW_RespWtime[2];		/* 送信結果受信完了待ち時間（単位＝秒）*/
										/* バイナリで格納 [0]=High byte, [1]=Low byte */
										/* 例）0x4E20 => [0]=0x4E, [1]=0x20 */
	uchar		system_select;			/* システム選択 */
	uchar		modem_exist;			/* モデムの有無 */
	uchar		foma_dopa;				/* 20H/'0' = DOPA   / '1' = FOMA, FOMAの場合 init_data に APN指定 ex>' 1sf.amano.co.jp' */
	uchar		yobi2[7];				/* 予備 */
	uchar		mobile_ark;				/* MobileArk選択 */
	uchar		Dpa_proc_knd;			/* Dopa 処理区分 [0]=常時通信なし, [1]=常時通信あり */
	uchar		Dpa_IP_h[12];			/* Dopa HOST局IPアドレス */
	uchar		Dpa_port_h[5];			/* Dopa HOST局ポート番号 */
	uchar		Dpa_IP_m[12];			/* Dopa 自局IPアドレス */
	uchar		Dpa_port_m[5];			/* Dopa 自局ポート番号 */
	uchar		Dpa_ppp_ninsho;			/* Dopa 着信時認証手順 */
	uchar		Dpa_nosnd_tm[4];		/* Dopa 無通信タイマー(秒) */
	uchar		Dpa_cnct_rty_tm[3];		/* Dopa 再発呼待ちタイマー(秒) */
	uchar		Dpa_com_wait_tm[3];		/* Dopa コマンド(ACK)待ちタイマー(秒) */
	uchar		Dpa_data_rty_tm[3];		/* Dopa データ再送待ちタイマー(分) */
	uchar		Dpa_discnct_tm[4];		/* Dopa TCPコネクション切断待ちタイマー(秒) */
	uchar		Dpa_cnct_rty_cn[3];		/* Dopa 再発呼回数 */
	uchar		Dpa_data_rty_cn;		/* Dopa データ再送回数(NAK,無応答) */
	uchar		Dpa_data_code;			/* Dopa HOST側電文コード [0]=文字コード, [1]=バイナリコード */
	uchar		yobi3[2];				/* 予備3 */

	uchar		alarm_non_notify[60];	/* アラーム非通知テーブルデータ１～３０ */
	uchar		error_non_notify[60];	/* エラー非通知　テーブルデータ１～３０ */
	uchar		interrupt_proc;			/* 中断要求時の処理 */
	uchar		interrupt_proc_block[2];/* 中断要求受信時の判定ブロック */

	uchar		Dpa_data_snd_rty_cnt;	// ＤｏＰａ下り回線パケット送信リトライ回数(無応答)
	uchar		Dpa_ack_wait_tm[3];		// ＤｏＰａ下り回線パケット応答待ち時間(ＡＣＫ/ＮＡＫ)
	uchar		Dpa_data_rcv_rty_cnt;	// ＤｏＰａ下り回線パケット受信リトライ回数(ＮＡＫ)
	uchar		Dpa_port_watchdog_tm[3];// ＤｏＰａ下り回線通信監視タイマ(秒)
	uchar		Dpa_port_m2[5];			// ＤｏＰａ下り回線 自局ポート番号
	uchar		netmask[2];				// ＸＰｏｒｔサブネットマスク
	uchar		gateway[12];			// デフォルトゲートウェイ

	uchar		yobi4[95];				/* 予備3 */
} RAU_CONFIG;

/****************************************************************/
/*		ＤｏｐａＲＡＵ設定電文構造体							*/
/****************************************************************/
typedef struct _DPA_RAU_CONFIG {
	uchar		rau_ip[12];				// 自局ＩＰアドレス
	uchar		rau_port[5];			// 【上り回線】自局ポート番号
	uchar		host_ip[12];			// 【上り回線】ＨＯＳＴ局ＩＰアドレス
	uchar		host_port[5];			// 遠隔ＨＯＳＴ局ポート番号
	uchar		ninsho;					// ＰＰＰ認証手順
	uchar		rev_user_id[21];		// ＨＯＳＴ局ＰＰＰ認証ＩＤ
	uchar		rev_password[15];		// ＨＯＳＴ局ＰＰＰ認証パスワード
	uchar		send_user_id[21];		// 自局ＰＰＰ認証ＩＤ
	uchar		send_password[15];		// 自局ＰＰＰ認証パスワード
	uchar		Dpa_proc_knd;			// ＤｏＰａ処理区分('0':常時通信なし, '1':常時通信あり)
	uchar		no_send_time[4];		// 無通信タイマー(秒)
	uchar		dail_retry_time[3];		// 再発呼待ちタイマー(秒)
	uchar		command_wait_time[3];	// 【上リ回線】パケット応答待ち時間（ＡＣＫ／ＮＡＫ）
	uchar		connect_retry[3];		// 再発呼回数
	uchar		data_retry;				// 【上リ回線】パケット送信リトライ回数（無応答）
	uchar		data_retry_time[3];		// データ再送待ちタイマー(分)
	uchar		disconect_time[4];		// ＴＣＰコネクション切断待ちタイマー(秒)
	uchar		Dpa_data_code;			// ＨＯＳＴ側電文コード('0':文字コード, '1':バイナリコード)
	uchar		data_snd_rty;			// 【下リ回線】パケット送信リトライ回数（無応答）
	uchar		ack_wait_tim[3];		// 【下リ回線】パケット応答待ち時間（ＡＣＫ／ＮＡＫ）
	uchar		data_rcv_rty;			// 【下リ回線】パケット受信リトライ回数（ＮＡＫ）
	uchar		port_watchdog_tim[3];	// 【下リ回線】受信監視タイマー
	uchar		rau_port_m2[5];			// 【下リ回線】自局ポート番号
	uchar		netmask[2];				// ＸＰｏｒｔサブネットマスク
	uchar		yobi[5];				// 予備
	uchar		dopa_cre_proc_kind;		// クレジットＤｏＰａ処理区分
	uchar		cre_ip[12];				// クレジット自局ＩＰアドレス
	uchar		cre_port[5];			// クレジット自局ポート番号
	uchar		cre_host_ip[12];		// クレジットＨＯＳＴ局ＩＰアドレス
	uchar		cre_host_port[5];		// クレジットＨＯＳＴ局ポート番号
	uchar		cre_data_retry[2];		// クレジットパケット送信リトライ回数
	uchar		cre_data_retry_time[2];	// クレジットパケット応答待ち時間
} DPA_RAU_CONFIG;

/////////////////////////////////////////////////////////////////
//			テーブル件数データ構造体
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking[2];		// 入庫データ			（ID20）
	uchar	out_parking[2];		// 出庫データ			（ID21）
	uchar	paid_data[2];		// 精算データ			（ID22）	ここでは22　23　を合算する｡
	uchar	total[2];			// T合計集計データ		（ID30～38・41）
	uchar	error[2];			// エラーデータ			（ID120）
	uchar	alarm[2];			// アラームデータ		（ID121）
	uchar	monitor[2];			// モニターデータ		（ID122）
	uchar	ope_monitor[2];		// 操作モニターデータ	（ID123）
	uchar	coin_total[2];		// コイン金庫集計データ	（ID131）
	uchar	money_toral[2];		// 紙幣金庫集計データ	（ID133）
	uchar	parking_num[2];		// 駐車台数・満車データ	（ID236）
	uchar	area_num[2];		// 区画台数・満車データ	（ID237）
	uchar	money_manage[2];	// 金銭管理データ		（ID126）(176 + 7 + 4) * 32
	uchar	turi_manage[2];		// 釣銭管理集計データ	（ID135）
	uchar	gtotal[2];			// GT合計集計データ		（ID42,43,45,46,49,53）
	uchar	rmon[2];			// 遠隔監視データ		（ID125）
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	uchar	dummy2[22];
	uchar	long_park[2];		// 長期駐車状態データ	（ID61）
	uchar	dummy2[20];
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
} RAU_TABLE_DATA_COUNT;

/////////////////////////////////////////////////////////////////
//			ニアフルデータ構造体
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking;			// 入庫データ			（ID20）
	uchar	out_parking;		// 出庫データ			（ID21）
	uchar	paid_data22;		// 精算データ			（ID22）
	uchar	paid_data23;		// 精算データ			（ID23）
	uchar	total;				// T合計集計データ		（ID30～38・41）
	uchar	error;				// エラーデータ			（ID120）
	uchar	alarm;				// アラームデータ		（ID121）
	uchar	monitor;			// モニターデータ		（ID122）
	uchar	ope_monitor;		// 操作モニターデータ	（ID123）
	uchar	coin_total;			// コイン金庫集計データ	（ID131）
	uchar	money_toral;		// 紙幣金庫集計データ	（ID133）
	uchar	parking_num;		// 駐車台数・満車データ	（ID236）
	uchar	area_num;			// 区画台数・満車データ	（ID237）
	uchar	turi_manage;		// 釣銭管理集計データ	（ID135）
	uchar	money_manage;		// 金銭管理データ		（ID126）
	uchar	rmon;				// 遠隔監視データ		（ID125）
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	uchar	dummy2[5];
	uchar	long_park;			// 長期駐車状態データ	（ID61）
	uchar	dummy2[4];
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
} RAU_NEAR_FULL;

/////////////////////////////////////////////////////////////////
//			通信要求データ
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking;			// 入庫データ			（ID20）
	uchar	out_parking;		// 出庫データ			（ID21）
	uchar	paid_data;			// 精算データ			（ID22 23）
	uchar	total;				// T合計集計データ		（ID30～38・41）
	uchar	error;				// エラーデータ			（ID120）
	uchar	alarm;				// アラームデータ		（ID121）
	uchar	monitor;			// モニターデータ		（ID122）
	uchar	ope_monitor;		// 操作モニターデータ	（ID123）
	uchar	coin_total;			// コイン金庫集計データ	（ID131）
	uchar	money_toral;		// 紙幣金庫集計データ	（ID133）
	uchar	parking_num;		// 駐車台数・満車データ	（ID236）
	uchar	area_num;			// 区画台数・満車データ	（ID237）
	uchar	money_manage;		// 金銭管理データ		（ID126）
	uchar	turi_manage;		// 釣銭管理集計データ	（ID135）
	uchar	rmon;				// 遠隔監視データ		（ID125）
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	uchar	dummy2[5];
	uchar	long_park;			// 長期駐車状態データ	（ID61）
	uchar	dummy2[4];
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
} RAU_SEND_REQUEST;

/////////////////////////////////////////////////////////////////
//			テーブルクリア要求データ
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking;			// 入庫データ			（ID20）
	uchar	out_parking;		// 出庫データ			（ID21）
	uchar	paid_data;			// 精算データ			（ID22 23）
	uchar	total;				// T合計集計データ		（ID30～38・41）
	uchar	error;				// エラーデータ			（ID120）
	uchar	alarm;				// アラームデータ		（ID121）
	uchar	monitor;			// モニターデータ		（ID122）
	uchar	ope_monitor;		// 操作モニターデータ	（ID123）
	uchar	coin_total;			// コイン金庫集計データ	（ID131）
	uchar	money_toral;		// 紙幣金庫集計データ	（ID133）
	uchar	parking_num;		// 駐車台数・満車データ	（ID236）
	uchar	area_num;			// 区画台数・満車データ	（ID237）
	uchar	money_manage;		// 金銭管理データ		（ID126）
	uchar	turi_manage;		// 釣銭管理集計データ	（ID135）
	uchar	rmon;				// 遠隔監視データ		（ID125）
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	uchar	dummy2[5];
	uchar	long_park;			// 長期駐車状態データ	（ID61）
	uchar	dummy2[4];
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
} RAU_CLEAR_REQUEST;

/////////////////////////////////////////////////////////////////
//			IBK制御データ
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	table_data_count;			// テーブル件数データ要求
	uchar	rau_config1;				// 遠隔IBK設定データ１要求
	uchar	rau_config2;				// 遠隔IBK設定データ２要求
	uchar	dummy2[15];
} RAU_CONTROL_REQUEST;

/////////////////////////////////////////////////////////////////
//			通信チェック[要求/結果]データ
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	NTpacket_len[2];		// ＮＴ－ＮＥＴ電文サイズ＋２ ※バッファ保存時のみの項目
	uchar	data_len[2];			// 共通部＋固有部の(０カット後)データ長
	uchar	data_len_zero_cut[2];	// 固有部０カット前のデータ長(５０)
	uchar	dummy;
	uchar	id1;					// ＩＤ１
	uchar	id2;					// ＩＤ２
	uchar	id3;					// ＩＤ３
	uchar	id4;					// ＩＤ４
	uchar	buffering_flag;			// データ保持フラグ       ↑共通部
// ------------------------------------------------------------
	uchar	SeqNo;					// シーケンシャル№       ↓固有部
	uchar	Parking[4];				// 駐車場№
	uchar	Model[2];				// 機種コード
	uchar	Destination[4];			// 送信先№
	uchar	Serial[6];				// 端末シリアル№
	uchar	OpeTimeDate[6];			// 処理年月日時分秒
	uchar	CheckNo;				// 通信チェック№
	union {
		struct {
			uchar	Kind;			// 要求種別
			uchar	TimeOut;		// タイムアウト
			uchar	dummy[24];
		} quest;
		struct {
			uchar	Code;			// 結果コード
			uchar	dummy[25];
		} sult;
	} Re;
	uchar	crc[2];					// ＣＲＣ ※バッファ保存時のみの項目
} RAU_COMMUNICATION_TEST;

/////////////////////////////////////////////////////////////////
//			Dopa用NTNETヘッダー
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
} RAU_NTDATA_HEADER;

/////////////////////////////////////////////////////////////////
//			NT-NET用　遠隔設定データ１
/////////////////////////////////////////////////////////////////
typedef struct {
	RAU_NTDATA_HEADER	nt_header;
	RAU_CONFIG			config_data1;
} RAU_CONFIG1;

/////////////////////////////////////////////////////////////////
//			NT-NET用　遠隔設定データ２
/////////////////////////////////////////////////////////////////
typedef struct {
	RAU_NTDATA_HEADER	nt_header;
	DPA_RAU_CONFIG		config_data2;
} RAU_CONFIG2;

#endif	// RAUIDPROC_H
