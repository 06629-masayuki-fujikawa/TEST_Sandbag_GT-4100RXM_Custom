/*[]----------------------------------------------------------------------[]*/
/*| Mifare関連制御                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"iodefine.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"mif.h"
#include	"prm_tbl.h"
#include	"lcd_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"

static	uchar	MIF_IsMntReq;

static short Ope_GT_Settei_Check_For_Mif( uchar gt_flg, ushort pno_syu );

/*[]----------------------------------------------------------------------[]*/
/*| Mifare制御処理                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpMif_ctl( ret )                                        |*/
/*| PARAMETER    : msg  : Message                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpMif_ctl( ushort msg )
{
	uchar	cmd, sts;

	cmd = 0;														// ｺﾏﾝﾄﾞ送信指示 0:未送信
	sts = 0;														// ｺﾏﾝﾄﾞｽﾃｰﾀｽ

	switch( msg ){
	case IBK_MIF_EVT_ERR:											// 通信異常
		cmd = MIF_mod;												// ﾃﾞｰﾀ再送
		break;

	case IBK_MIF_A2_NG_EVT:											// MIF:ｶｰﾄﾞﾃﾞｰﾀ読出しNG
		switch( MIF_ENDSTS.sts1 ){
		case 0x01:													// ｺﾏﾝﾄﾞIDｴﾗｰ
		case 0x02:													// ｾｸﾀ番号NG
		case 0x03:													// ﾌﾞﾛｯｸ数指定NG
		case 0x04:													// 書込みﾃﾞｰﾀ無し
		case 0x05:													// ｷｰ指定NG
		case 0x10:													// ｶｰﾄﾞ未検知
		case 0x11:													// 読出し中
		case 0x12:													// 書込み中
		case 0x13:													// KEY登録中
		case 0x14:													// 処理中
		case 0x15:													// 処理終了待ち
		case 0x16:													// 認証ｷｰｴﾗｰ
			break;
		case 0x20:													// Limit Time Over
		case 0x21:													// NAK Retry Over
		case 0x22:													// Time Out
		case 0x23:													// Bcc Err
		case 0x30:													// ｶｰﾄﾞ無し
		case 0x40:													// ｾﾝｼﾝｸﾞNG
		case 0x50:													// Read Verify NG
		case 0x60:													// Write Verify NG
		case 0xf0:													// その他のｴﾗｰ
			if( !MIF_IsMntReq && (OPECTL.Ope_mod != 2)){
				if( MIF_mod == 0xA2 ){								// ｶｰﾄﾞｾﾝｽ&ﾃﾞｰﾀ読出しｺﾏﾝﾄﾞ(A2)送信済み?
					cmd = 0xA2;										// ｶｰﾄﾞｾﾝｽ&ﾃﾞｰﾀ読出しｺﾏﾝﾄﾞ(A2)再送
				}
			}
			break;
		}
		break;

	case IBK_MIF_A3_NG_EVT:											// 処理中止NG
	case IBK_MIF_A3_OK_EVT:											// 処理中止OK
		MIF_mod = 0;												// ｺﾏﾝﾄﾞ状態(ｱｲﾄﾞﾙ)
		break;

	case IBK_MIF_A4_OK_EVT:											// MIF:ｶｰﾄﾞﾃﾞｰﾀ書込みOK
		if( !MIF_IsMntReq ){
			if( OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3 ){
				PassExitTimeTblDelete( MifCard.pk_no, MifCard.pas_id );
			}
		}
		if( MIF_mod == 0xA4 ){										// 書込み(A4)送信済み?
			MIF_mod = 0xA2;											// ｺﾏﾝﾄﾞ状態をｶｰﾄﾞｾﾝｽ&ﾃﾞｰﾀ読出し(A2)状態とする
		}
		break;

	case IBK_MIF_A4_NG_EVT:											// MIF:ｶｰﾄﾞﾃﾞｰﾀ書込みNG
		if( !MIF_IsMntReq ){
			if( OPECTL.Ope_mod == 3 ){		//精算完了時のみ登録
				PassExitTimeTblWrite( MifCard.pk_no, MifCard.pas_id, &MifCard.ext_tm );
			}
		}
		switch( MIF_ENDSTS.sts1 ){
		case 0x60:													// Write Verify NG
			err_chk( ERRMDL_MIFARE, ERR_MIF_WRT_VERIFY, 2, 0, 1 );
			cmd = 0xA4;												// 書込み(A4)再送
			break;
		default:													// Write NG(Verify以外)
			err_chk( ERRMDL_MIFARE, ERR_MIF_WRT_NG, 2, 0, 1 );
			break;
		}
		break;

	default:
		break;
	}
	/*--------------*/
	/* ｺﾏﾝﾄﾞ送信	*/
	/*--------------*/
	if( cmd ){
		OpMif_snd( cmd, sts );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifareｺﾏﾝﾄﾞ送信処理                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpMif_snd( cmdid, stat )                                |*/
/*| PARAMETER    : cmdid = 送信するｺﾏﾝﾄﾞID                                 |*/
/*|                stat  = ｽﾃｰﾀｽ(ﾃｽﾄｺﾏﾝﾄﾞのみで使用)                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpMif_snd( uchar cmdid, uchar stat )
{
	MIF_IsMntReq = 0;
		MIF_mod = cmdid;											// ｺﾏﾝﾄﾞ状態更新
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifareｺﾏﾝﾄﾞ送信処理(for 定期発行画面)                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpMif_snd2( cmdid, stat )                               |*/
/*| PARAMETER    : cmdid = 送信するｺﾏﾝﾄﾞID                                 |*/
/*|                stat  = ｽﾃｰﾀｽ(ﾃｽﾄｺﾏﾝﾄﾞのみで使用)                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpMif_snd2( uchar cmdid, uchar stat )
{
	CMN_UNUSED_PARAMETER(stat);
	MIF_IsMntReq = 1;
		MIF_mod = cmdid;											// ｺﾏﾝﾄﾞ状態更新
	if( cmdid == 0xA2 ){
		stat = 1;		// 検知待ち時間無制限
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifareﾘｰﾄﾞ                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MifareDataChk( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = 駐車場№ｴﾗｰ                                  |*/
/*|                       2 = ﾃﾞｰﾀ異常                                     |*/
/*|                       3 = 期限切れ                                     |*/
/*|                       4 = 入出庫ｴﾗｰ                                    |*/
/*|                       5 = 無効定期                                     |*/
/*|                       6 = 期限前                                       |*/
/*|                       9 = 二重使用ｴﾗｰ                                  |*/
/*|                      13 = 種別規定外                                   |*/
/*|                      25 = 車種ｴﾗｰ                                      |*/
/*|                      26 = 設定ｴﾗｰ                                      |*/
/*|                      27 = 料金種別ｴﾗｰ                                  |*/
/*|                      29 = 問合せｴﾗｰ                                    |*/
/*|                      30 = 精算順序ｴﾗｰ                                  |*/
/*|                      99 = HOSTへ問合せ                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const char mgtype[4] = {6,1,2,3};
short	MifareDataChk( void )
{
	short	ret;
	short	wk;
	ushort	us_sday;
	ushort	us_eday;
	short	w_syasyu;
	date_time_rec	extim;
	short	s_ptbl;
	ushort	us_psts[3];
	char	c_prm;
	char	c_ptyp;
	ulong	ul_AlmPrm;
	uchar	uc_GtAlmPrm[10];
	short	data_adr;		// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ
	char	data_pos;		// 使用可能料金種別のﾃﾞｰﾀ位置
	uchar	KigenCheckResult;

	int		i;
	t_MIF_CARDFORM	WK_MifareCard;
	char	GT_Mifare_RCRC_NG = 0;						// 0:アンチパスON  1:アンチパスOFF
	ret = 0;

	if( 0 == OPECTL.ChkPassSyu ){	// Mifareカードデータ受信
		if(ret == 0){
			wk  = Mifare_WrtNgDataSearch( &MIF_CARD_SID[0] );
			if( 0xffff != wk ){	// あり
				// ベースはRead時のデータとする。
				memcpy( &WK_MifareCard, &MIF_WriteNgCard_ReadData.data[wk].Data, sizeof(t_MIF_CARDFORM) );
				// 有効終了年月日で比較
				us_sday = dnrmlzm( (short)MIF_WriteNgCard_ReadData.data[wk].Data.limit_e.Year,
								 (short)MIF_WriteNgCard_ReadData.data[wk].Data.limit_e.Mon,
								 (short)MIF_WriteNgCard_ReadData.data[wk].Data.limit_e.Day );	// バッファ有効終了日ｾｯﾄ

				us_eday = dnrmlzm( (short)MifCard.limit_e.Year, (short)MifCard.limit_e.Mon, (short)MifCard.limit_e.Day );	// カード有効終了日ｾｯﾄ

				if(us_sday == us_eday){
					WK_MifareCard.limit_e = MIF_WriteNgCard_ReadData.data[wk].Data.limit_e;		// カード有効終了年月日
					WK_MifareCard.kosin_ymd = MIF_WriteNgCard_ReadData.data[wk].Data.kosin_ymd;	// 更新年月日
				}else if(us_sday < us_eday){
					WK_MifareCard.limit_e = MifCard.limit_e;		// カード有効終了年月日
					WK_MifareCard.kosin_ymd = MifCard.kosin_ymd;	// 更新年月日
				}else{
					WK_MifareCard.limit_e = MIF_WriteNgCard_ReadData.data[wk].Data.limit_e;		// カード有効終了年月日
					WK_MifareCard.kosin_ymd = MIF_WriteNgCard_ReadData.data[wk].Data.kosin_ymd;	// 更新年月日
				}
				
				memcpy( &MifCard, &WK_MifareCard, sizeof(t_MIF_CARDFORM) );
				PayData.teiki.id = 0;	// 二重使用エラー回避
				ret = 0;				// カードReadデータ正常セット
			}
		}else{
			if(ret == 1){					// CRCエラー
				if(MifCard.type == 0x75){								// GTフォーマット
					memset(uc_GtAlmPrm,0x00,10);						// 0クリア
					memcpy(&uc_GtAlmPrm[0],&MifCard.pk_no,4);			// 駐車場Noセット
					memcpy(&uc_GtAlmPrm[4],&MifCard.pas_id,2);			// 定期券IDセット
					alm_chk2( ALMMDL_SUB2, ALARM_GT_MIFARE_READ_CRC_ERR, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
					if(prm_get( COM_PRM, S_TIK, 25, 1, 1 ) == 1){
						GT_Mifare_RCRC_NG = 1;				// ブロック３書き込み不良時アンチパスOFFとする。
						ret = 0;							// カードReadデータ正常セット
					}
				}
			}
			wk  = Mifare_WrtNgDataSearch( &MIF_CARD_SID[0] );
				// 書込み失敗テーブルに登録されているか検索
			if( 0xffff != wk ){	// あり
				// 前回Read値を有効とする
				memcpy( &MifCard, &MIF_WriteNgCard_ReadData.data[wk].Data, sizeof(t_MIF_CARDFORM) );
				PayData.teiki.id = 0;	// 二重使用エラー回避
				ret = 0;				// カードReadデータ正常セット
			}
		}
		if( 0 == ret ){	// 受信データ正常（変換完了）
			memcpy( &MIF_LastReadCardData.Sid,  &MIF_CARD_SID[0], 4);
			memcpy( &MIF_LastReadCardData.Data, &MifCard,
				sizeof(t_MIF_CARDFORM) );
																			// Ope用 エリアに保存
		}
		else{															// エラー (CRC-NGなど）
			if(ret == 1){									// CRCエラー
				if(MifCard.type == 0x75){
					return( 47 );							// GTフォーマット
				}else{
					return( 2 );							// APSフォーマット
				}
			}else if(ret == 2){								// IDエラー
				return( 8 );
			}else{
				return( 2 );
			}
		}
	}

	ret = 0;
	for( ; ; ){
		if( (MifCard.type != 0x70) && (MifCard.type != 0x75) ){									// ｶｰﾄﾞﾀｲﾌﾟMifare定期?
			ret = 13;												// 種別規定外
			break;
		}
		if(MifCard.type == 0x70){								// APSフォーマット
			ul_AlmPrm = (ulong)MifCard.pk_no;
			ul_AlmPrm *= 100000L;								// 定期券駐車場set
			ul_AlmPrm += (ulong)MifCard.pas_id;					// 定期券IDset(1～12000)
		}else{													// GTフォーマット
			memset(uc_GtAlmPrm,0x00,10);						// 0クリア
			memcpy(&uc_GtAlmPrm[0],&MifCard.pk_no,4);			// 駐車場Noセット
			memcpy(&uc_GtAlmPrm[4],&MifCard.pas_id,2);			// 定期券IDセット
		}
		if( 0L == (ulong)MifCard.pk_no ){
			ret = 1;											// 駐車場№ｴﾗｰ
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{												// GTフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}
		if(( prm_get( COM_PRM, S_SYS, 70, 1, 6 ) == 1 )&&				// 基本定期券使用可
		   ( CPrmSS[S_SYS][1] == (ulong)MifCard.pk_no )){ 				// 基本駐車場№?
			c_ptyp = KIHON_PKNO;										// 基本
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 1 ) == 1 )&&			// 拡張1定期券使用可
				( CPrmSS[S_SYS][2] == (ulong)MifCard.pk_no )){			// 拡張1駐車場№?
			c_ptyp = KAKUCHOU_1;										// 拡張1
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 2 ) == 1 )&&			// 拡張2定期券使用可
				( CPrmSS[S_SYS][3] == (ulong)MifCard.pk_no )){			// 拡張2駐車場№?
			c_ptyp = KAKUCHOU_2;										// 拡張2
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 3 ) == 1 )&&			// 拡張3定期券使用可
				( CPrmSS[S_SYS][4] == (ulong)MifCard.pk_no )){			// 拡張3駐車場№?
			c_ptyp = KAKUCHOU_3;										// 拡張3
		}
		else{
			ret = 1;													// 駐車場№ｴﾗｰ
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{												// GTフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}

		ret = Ope_GT_Settei_Check_For_Mif( (MifCard.type == 0x75 ? 1:0), (ushort)c_ptyp );
		if( ret == 1 ){
			if( MifCard.type == 0x70 ){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{												// GTフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			ret = 33;
			break;
			
		}else if( ret == 2 ){
			ret = 1;											// 駐車場№ｴﾗｰ
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{												// GTフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}			

		if( ((MifCard.type == 0x75) && ((GTF_PKNO_LOWER > MifCard.pk_no) || (MifCard.pk_no > GTF_PKNO_UPPER))) ||
			((MifCard.type == 0x70) && ((APSF_PKNO_LOWER > MifCard.pk_no) || (MifCard.pk_no > APSF_PKNO_UPPER))) ){//駐車場No範囲チェック

			ret = 1;												// 駐車場№ｴﾗｰ
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{												// GTフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}

		s_ptbl = ReadPassTbl( MifCard.pk_no, MifCard.pas_id, us_psts );
		if( s_ptbl == -1 ){											// ｴﾗｰ(数値異常)
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}
		if( us_psts[0] ){											// 無効登録
			ret = 5;												// 無効ｴﾗｰ
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_MUKOU_PASS_USE, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}

		if( (short)prm_get( COM_PRM,S_PAS,(short)(5+10*(MifCard.pas_knd-1)),1,1 ) == 1 ){	// 期限切れ時受付する設定
			c_prm = (char)prm_get( COM_PRM,S_PAS,(short)(5+10*(MifCard.pas_knd-1)),1,2 );	// 無期限設定Get
		}else{
			c_prm = 0;
		}

		if( c_prm == 1 || c_prm == 2 ){								// 無期限 or 開始日無期限
			us_sday = dnrmlzm( 1980, 3, 1 );						// 1980年3月1日
		}else{
			if( chkdate( (short)MifCard.limit_s.Year,				// 有効開始日ﾁｪｯｸNG?
			             (short)MifCard.limit_s.Mon,
			             (short)MifCard.limit_s.Day ) ){
				return( 2 );										// ﾃﾞｰﾀ異常
			}
			us_sday = dnrmlzm( (short)MifCard.limit_s.Year, (short)MifCard.limit_s.Mon, (short)MifCard.limit_s.Day );	// 有効開始日ｾｯﾄ
		}

		CRD_DAT.PAS.std_end[0] = (char)(MifCard.limit_s.Year % 100);
		CRD_DAT.PAS.std_end[1] = MifCard.limit_s.Mon;
		CRD_DAT.PAS.std_end[2] = MifCard.limit_s.Day;

		if( c_prm == 1 || c_prm == 3 ){								// 無期限 or 終了日無期限
			us_eday = dnrmlzm( 2079, 12, 31 );						// 2079年12月31日
		}else{
			if( chkdate( (short)MifCard.limit_e.Year,				// 有効終了日ﾁｪｯｸNG?
			             (short)MifCard.limit_e.Mon,
			             (short)MifCard.limit_e.Day ) ){
				return( 2 );										// ﾃﾞｰﾀ異常
			}
			us_eday = dnrmlzm( (short)MifCard.limit_e.Year, (short)MifCard.limit_e.Mon, (short)MifCard.limit_e.Day );	// 有効終了日ｾｯﾄ
		}

		CRD_DAT.PAS.std_end[3] = (char)(MifCard.limit_e.Year % 100);
		CRD_DAT.PAS.std_end[4] = MifCard.limit_e.Mon;
		CRD_DAT.PAS.std_end[5] = MifCard.limit_e.Day;

		if( !rangechk( 1, 15, MifCard.pas_knd ) ||					// 定期種別範囲外
			!rangechk( 0, 3, ( MifCard.io_stat & 0x0F ) ) )					// ｽﾃｰﾀｽ範囲外
		{
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}

		if( ryo_buf.syubet < 6 ){
			// 精算対象の料金種別A～F(0～5)
			data_adr = 10*(MifCard.pas_knd-1)+9;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
			data_pos = (char)(6-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
		}
		else{
			// 精算対象の料金種別G～L(6～11)
			data_adr = 10*(MifCard.pas_knd-1)+10;					// 使用可能料金種別のﾃﾞｰﾀｱﾄﾞﾚｽ取得
			data_pos = (char)(12-ryo_buf.syubet);					// 使用可能料金種別のﾃﾞｰﾀ位置取得
		}
		if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){		// 使用不可設定？
			ret = 25;												// 他の車種の券
			break;
		}

		if( PayData.teiki.id != 0 ){								// 二重使用?
			ret = 9;												// 二重使用ｴﾗｰ
			break;
		}

		if( CPrmSS[S_TIK][9] ){										// n分ﾁｪｯｸ有り?
			if( (PassIdBackupTim) &&								// n分ﾁｪｯｸ起動中?
				(PassPkNoBackup == MifCard.pk_no) &&
				(PassIdBackup   == MifCard.pas_id) ){				// 前回と同一ｶｰﾄﾞ
				ret = 9;											// 二重使用ｴﾗｰ(n分ﾁｪｯｸｴﾗｰ)
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_N_MINUTE_RULE, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_N_MINUTE_RULE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
				break;
			}
		}

		if(( (MifCard.io_stat & 0x0F) == 0 )&&								// ﾊﾞｰｼﾞﾝ?
		   ( CPrmSS[S_TIK][8] == 0 )){								// 初回定期読まない設定?
			ret = 26;												// 初回定期使用不可ｴﾗｰ
			break;
		}

		if( us_sday > us_eday ){									// 有効期限ﾃﾞｰﾀ異常(開始＞終了)
			ret = 13;												// 種別規定外ｴﾗｰ
			break;
		}

		/** 定期有効期限チェック処理(チェック方式をNT-7700に合わせる) **/
		KigenCheckResult = Ope_PasKigenCheck( us_sday, us_eday, (short)MifCard.pas_knd, CLK_REC.ndat, CLK_REC.nmin );
											// 定期有効期限チェック（戻り：0=期限開始日より前，1=有効期限内，2=期限終了日より後）

		if( (1 != KigenCheckResult) &&								// 期限切れ
			(prm_get( COM_PRM,S_PAS,(short)(5+10*(MifCard.pas_knd-1)),1,1 ) == 0 ) ){	// 期限切れ時受付しない設定?

			/*** 期限切れで期限切れ定期受付しない設定の時 ***/
			if( 0 == KigenCheckResult ){							// 期限前
				ret = 6;											// 期限前ｴﾗｰ
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
				break;
			}
			else{													// 期限切れ
				ret = 3;											// 期限切れｴﾗｰ
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
				break;
			}
			break;
		}

		w_syasyu = 0;
		c_prm = (char)CPrmSS[S_PAS][1+10*(MifCard.pas_knd-1)];		// 使用目的設定Get
		if( !rangechk( 1, 14, c_prm ) ){							// 定期使用目的設定範囲外
			if (c_prm == 0) {										// 使用目的：未使用
				ret = 26;											// 設定ｴﾗｰ
			}
			else {
				ret = 13;											// 種別規定外
			}
			if(MifCard.type == 0x70){							// APSフォーマット
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
			}
			break;
		}
		if( rangechk( 3, 14, c_prm ) ){								// 定期車種切換?
			w_syasyu = c_prm - 2;									// 車種切換用車種ｾｯﾄ
			if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(w_syasyu-1)),2,5 ) == 0L ) {	// 車種設定なし
				ret = 27;											// 料金種別ｴﾗｰ
				break;
			}
			if(( ryo_buf.waribik )||								// 割引済み?
			   ( ryo_buf.zankin == 0 ))								// 残金0円の時
			{
				ret = 30;											// 精算順番ｴﾗｰ
				break;
			}
		}
		// 減額精算後の定期券は無効
		i = is_paid_remote(&PayData);
		if (i >= 0 &&
		   ( PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU || 
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
//// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
//		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) || 
//// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応
		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE && !vl_frs.antipassoff_req)) ) {
			ret = 8;
			break;
		}
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
		i = is_ParkingWebFurikae( &PayData );
		if (i >= 0 &&  ( PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) ){
			ret = 8;
			break;
		}
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応
		if(	(OPECTL.Pay_mod == 2)&&						// 修正精算
			(vl_frs.antipassoff_req)&&					// 修正元定期券使用
			(prm_get(COM_PRM, S_TYP, 98, 1, 4)==1)){	// 同一定期ﾁｪｯｸする
			if( syusei[vl_frs.lockno-1].tei_id != CRD_DAT.PAS.cod ){
				ret = 13;											// 種別規定外
				break;
			}
		}
		ryo_buf.pass_zero = 0;										// 定期券強制0円精算要求なし
		if( chk_for_inquiry(1) ) {									// 問合せる?
			if( !ERR_CHK[mod_ntibk][1] ){							// NTNET IBK 通信正常?
				OPECTL.ChkPassSyu = 2;								// 定期券問合せ中券種2=Mifare定期券
				OPECTL.ChkPassPkno = (ulong)MifCard.pk_no;			// 定期券問合せ中駐車場№
				OPECTL.ChkPassID = MifCard.pas_id;					// 定期券問合せ中定期券ID
				// ここでは送信要求をかけない。opemainのELE_EVT_STOPで電子媒体の停止を待ってから送信する
				ret = 99;
				break;
			}else{
				OPECTL.ChkPassSyu = 0xfe;							// 通信不良
				wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );		// 通信不良時の設定
				if( wk == 0 ){										// NGで使用不可
					ret = 29;										// 通信不良or応答ﾀｲﾑｱｳﾄ
					break;
				}else if( wk == 1 ){								// 強制0円
					ryo_buf.pass_zero = 1;							// 定期券強制0円精算要求あり
				}
			}
		}

		if( PassExitTimeTblRead( MifCard.pk_no, MifCard.pas_id, &extim ) ){
			// 前回書込み失敗したので出庫時刻を前回書込むはずだった時刻にする。
			memcpy( &MifCard.ext_tm, &extim, sizeof( date_time_rec ));
		}

		if( OPECTL.ChkPassSyu == 2 ){								// 定期券問合せ後の問合せ結果受信
			memcpy( &MifCard.ext_tm, &PassChk.OutTime, sizeof( date_time_rec ) );	// 処理月日時分を受信ﾃﾞｰﾀに更新
		}
		else if( OPECTL.ChkPassSyu == 0xff ){						// 定期券問合せ後の問合せ結果受信ﾀｲﾑｱｳﾄ
			wk = (short)prm_get( COM_PRM,S_NTN,36,1,1 );			// 問合せ応答ﾀｲﾑｱｳﾄ時の設定
			if( wk == 0 ){											// NGで使用不可
				ret = 29;											// 通信不良or応答ﾀｲﾑｱｳﾄ
				break;
			}else if( wk == 1 ){									// 強制0円
				ryo_buf.pass_zero = 1;								// 定期券強制0円精算要求あり
			}
		}
		if(( DO_APASS_CHK )&&
		   ( prm_get( COM_PRM,S_PAS,(short)(2+10*(MifCard.pas_knd-1)),1,1 ) )&&	// 入出ﾁｪｯｸする設定?
		   ( OPECTL.ChkPassSyu != 0xff )&&							// 定期券問合せ後の問合せ結果受信ﾀｲﾑｱｳﾄ以外
		   ( GT_Mifare_RCRC_NG == 0 ) &&							// Mifare読取りCRCエラー発生時入出チェックする?
		   ( OPECTL.ChkPassSyu != 0xfe )){							// 通信不良
			if((MifCard.io_stat & 0x0f) != 0 ){
				wk = CLK_REC.year;
				us_sday = dnrmlzm( wk, (short)MifCard.ext_tm.Mon, (short)MifCard.ext_tm.Day );	// ｶｰﾄﾞ月日ﾉｰﾏﾗｲｽﾞ

				if( us_sday > CLK_REC.ndat ){
					wk--;											// 年ﾏｲﾅｽ
					us_sday = dnrmlzm( wk, (short)MifCard.ext_tm.Mon, (short)MifCard.ext_tm.Day );	// ｶｰﾄﾞ月日ﾉｰﾏﾗｲｽﾞ
				}
				wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );

				if( us_sday > wk ){									// ｶｰﾄﾞ月日 > 入庫月日?
					ret = 4;										// 入出ｴﾗｰ
					break;
				}
				if( us_sday == wk ){
					wk = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );

					us_sday = tnrmlz( 0, 0, (short)MifCard.ext_tm.Hour, (short)MifCard.ext_tm.Min );
					if( us_sday > wk ){
						ret = 4;									// 入出ｴﾗｰ
						break;
					}
				}
			}
		}
		if( KaisuuWaribikiGoukei || c_pay || PayData_Sub.pay_ryo ){ // ﾌﾟﾘﾍﾟ・回数券使用時は定期券の使用を不可とする
			ret = 9;												// 二重使用ｴﾗｰ
		}
		break;
	}

	if( ret == 4 ){													// 入出ｴﾗｰ?
		for( wk=0; wk<TKI_CYUSI_MAX; wk++ ){
			if( MifCard.pas_id == tki_cyusi.dt[wk].no &&			// 中止ﾃﾞｰﾀと一致?
				(ulong)MifCard.pk_no == tki_cyusi.dt[wk].pk ){
				tkcyu_ichi = (char)(wk + 1);						// 中止読取位置
				ret = 0;											// ﾁｪｯｸOK
				break;
			}
		}
		if( ret == 4 ){
			if( chk_for_inquiry(2) ) {								// ｱﾝﾁﾊﾟｽNGの時のみ問合せる?
				if( !ERR_CHK[mod_ntibk][1] ){						// NTNET IBK 通信正常?
					OPECTL.ChkPassSyu = 2;							// 定期券問合せ中券種2=Mifare定期券
					OPECTL.ChkPassPkno = (ulong)MifCard.pk_no;		// 定期券問合せ中駐車場№
					OPECTL.ChkPassID = MifCard.pas_id;				// 定期券問合せ中定期券ID
					// ここでは送信要求をかけない。opemainのELE_EVT_STOPで電子媒体の停止を待ってから送信する
					ret = 99;
				}else{
					wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );	// 通信不良時の設定
					if( wk == 0 ){									// NGで使用不可
						ret = 29;									// 通信不良or応答ﾀｲﾑｱｳﾄ
					}else if( wk == 1 ){							// 強制0円
						ryo_buf.pass_zero = 1;						// 定期券強制0円精算要求あり
						ret = 0;
					}else if( wk == 2 ){							// ｶｰﾄﾞ時刻で精算
						ret = 0;
					}
				}
			}
		}
	}

	if( ret == 4){		//入出ｴﾗｰ?
		if(MifCard.type == 0x70){							// APSフォーマット
			alm_chk2( ALMMDL_SUB2, ALARM_ANTI_PASS_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
		}else{
			alm_chk2( ALMMDL_SUB2, ALARM_GT_ANTI_PASS_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ｷｬﾝｾﾚｰｼｮﾝｱﾗｰﾑ登録
		}
	}

	if( ret == 99 && MifStat == MIF_WROTE_FAIL ){								// 書込み失敗後の再ﾀｯﾁ
		ret = 0;											// 正常終了とする
	}

	if( ret != 99 ){												// 問合せしていない?
		OPECTL.ChkPassSyu = 0;										// 定期券問合せ中券種ｸﾘｱ
		OPECTL.ChkPassPkno = 0L;									// 定期券問合せ中駐車場№ｸﾘｱ
		OPECTL.ChkPassID = 0L;										// 定期券問合せ中定期券IDｸﾘｱ
		blink_end();												// 点滅終了
	}

	if( ret == 0 ){													// ｶｰﾄﾞOK?
		if( w_syasyu ){
			vl_now = V_SYU;											// 種別切換
			syashu = (char)w_syasyu;								// 車種
		}else{														// 車種切換でない
			vl_now = V_TSC;											// 定期券(駐車券併用有り)
		}
		// 挿入定期ﾃﾞｰﾀを共通ｴﾘｱにｾｰﾌﾞ （vl_paschg()関数をｺｰﾙするため）
		CRD_DAT.PAS.knd = MifCard.pas_knd;				// 定期券種別
		CRD_DAT.PAS.sts = MifCard.io_stat & 0x0f;		// 定期券ｽﾃｰﾀｽ(読取時)
		CRD_DAT.PAS.cod = MifCard.pas_id;				// 定期券id
		memcpy( CRD_DAT.PAS.trz, &MifCard.ext_tm.Mon, 4 );		// 処理月日時分
		CRD_DAT.PAS.pno = MifCard.pk_no;				// 駐車場番号

		// Mifares専用関数として使用していたvl_mifpaschgは使用しないようにし、vl_paschgを共通関数として使用する
		// 用に修正する
		vl_paschg();

		// 挿入定期ﾃﾞｰﾀを一時保存ｴﾘｱにｾｰﾌﾞ（OK確定後 PatDataへ登録）
		InTeiki_PayData_Tmp.syu 		 = (uchar)CRD_DAT.PAS.knd;	// 定期券種別
		InTeiki_PayData_Tmp.status 		 = (uchar)CRD_DAT.PAS.sts;	// 定期券ｽﾃｰﾀｽ(読取時)
		InTeiki_PayData_Tmp.id 			 = CRD_DAT.PAS.cod;			// 定期券id
		InTeiki_PayData_Tmp.pkno_syu 		 = c_ptyp;			// 定期券駐車場Ｎｏ．種別 (0-3:基本,拡張1-3)
		InTeiki_PayData_Tmp.update_mon 	 	 = 0;						// 更新月数
		InTeiki_PayData_Tmp.s_year 		 = MifCard.limit_s.Year;					// 有効期限（開始：年）
		InTeiki_PayData_Tmp.s_mon 		 = MifCard.limit_s.Mon;		// 有効期限（開始：月）
		InTeiki_PayData_Tmp.s_day 		 = MifCard.limit_s.Day;		// 有効期限（開始：日）
		InTeiki_PayData_Tmp.e_year 		 = MifCard.limit_e.Year;					// 有効期限（終了：年）
		InTeiki_PayData_Tmp.e_mon 		 = MifCard.limit_e.Mon;		// 有効期限（終了：月）
		InTeiki_PayData_Tmp.e_day 		 = MifCard.limit_e.Day;		// 有効期限（終了：日）
		memcpy( InTeiki_PayData_Tmp.t_tim, CRD_DAT.PAS.trz, 4 );	// 処理月日時分
		InTeiki_PayData_Tmp.update_rslt1 = 0;						// 定期更新精算時の更新結果			（　OK：更新成功　／　NG：更新失敗　）
		InTeiki_PayData_Tmp.update_rslt2 = 0;						// 定期更新精算時のラベル発行結果	（　OK：正常発行　／　NG：発行不良　）
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare書込み処理                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MifareDataWrt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	MifareDataWrt( void )
{

	MifCard.io_stat &= 0xF0;											// ｶｰﾄﾞ入出庫ｽﾃｰﾀｽ更新(クリア)
	MifCard.io_stat |= 0x01;											// ｶｰﾄﾞ入出庫ｽﾃｰﾀｽ更新(出庫)
	memcpy( &MifCard.ext_tm.Year, &CLK_REC, sizeof( date_time_rec ));	// 出庫時刻更新

	OpMif_snd( 0xA4, 0 );											// Mifare書込み
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare書込み処理(for 定期発行画面)                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MifareDataWrt2( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	MifareDataWrt2( void )
{
	OpMif_snd2( 0xA4, 0 );											// Mifare書込み
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare書込み失敗時のデータ更新                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : Request	要求処理									   |*/
/*|							1=全ｸﾘｱ、2=1件削除、3=1件登録				   |*/
/*|				   pData    Request毎に内容が異なる						   |*/
/*|							Request=1 → 意味なし（0LでOK）				   |*/
/*|							Request=2 → 削除するSIDを指定				   |*/
/*|										 他の内容は指定不要				   |*/
/*|							Request=3 → 登録する内容（全項目指定必要）	   |*/
/*|										 同一SIDｶｰﾄﾞ情報がある場合は	   |*/
/*|										 旧ﾃﾞｰﾀを削除して新規登録しなおす  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	・本関数ではﾜｰｸｴﾘｱに情報をｾｯﾄするのみで、実ﾃﾞｰﾀの更新は次の関数で行う。|*/
/*|	・停電に関する動作は、既存ﾃﾞｰﾀを壊さないようにすることを主とする。	   |*/
/*|	  ﾜｰｸｴﾘｱ登録前に電源が落ちた場合は保証しない。						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-11-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Mifare_WrtNgDataUpdate( uchar Request, void *pVoid )
{
	t_MIF_CardData *pData = (t_MIF_CardData*)pVoid;

	ushort	Index;

	switch( Request ){
	case	1:														// 全ｸﾘｱ
		/* ﾜｰｸｴﾘｱにﾃﾞｰﾀｾｯﾄ */
		MIF_WriteNgCard_ReadData.wk_count = 0;
		memset( &MIF_WriteNgCard_ReadData.wk_data[0], 0, sizeof(t_MIF_CardData)*MIF_WRITE_NG_BUF_COUNT );
		/* 実ﾃﾞｰﾀ更新(停電保障) */
		MIF_WriteNgCard_ReadData.f_Phase = 0x55;					// ﾜｰｸｴﾘｱにﾃﾞｰﾀｾｯﾄ完了
		Mifare_WrtNgDataUpdate_Exec();
		break;

	case	2:														// 1件削除指示
		/* pData->Sid[] に対象ﾃﾞｰﾀあり */
		Index = Mifare_WrtNgDataSearch( &(pData->Sid[0]) );			// 同一SID検索
		if( 0xffff == Index )										// 同一SIDなし
			return;													// 終了

		/* 消去するﾃﾞｰﾀの前までをﾜｰｸﾊﾞｯﾌｧにcopy */
		if( 0 != Index ){											// 先頭ﾃﾞｰﾀではない
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
					&MIF_WriteNgCard_ReadData.data[0], sizeof(t_MIF_CardData)*(Index) );
		}
		/* 消去するﾃﾞｰﾀを抜かして 最後までをﾜｰｸﾊﾞｯﾌｧにcopy */
		if( (MIF_WRITE_NG_BUF_COUNT - 1) != Index ){				// 最後のﾃﾞｰﾀではない
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[Index], 
					&MIF_WriteNgCard_ReadData.data[Index+1], 
						sizeof(t_MIF_CardData)*((MIF_WRITE_NG_BUF_COUNT - 1) - Index) );
		}

		/* 今までFull件数だったら末尾を0ｸﾘｱする */
		if( MIF_WRITE_NG_BUF_COUNT <= MIF_WriteNgCard_ReadData.count ){	// Buffer full
			memset( &MIF_WriteNgCard_ReadData.wk_data[MIF_WRITE_NG_BUF_COUNT - 1], 0, sizeof(t_MIF_CardData) );
		}

		/* 登録件数 - 1 */
		MIF_WriteNgCard_ReadData.wk_count = MIF_WriteNgCard_ReadData.count - 1;

		/* 実ﾃﾞｰﾀ更新(停電保障) */
		MIF_WriteNgCard_ReadData.f_Phase = 0x55;					// ﾜｰｸｴﾘｱにﾃﾞｰﾀｾｯﾄ完了
		Mifare_WrtNgDataUpdate_Exec();
		break;

	case	3:														// 1件登録
		/* 初めに同一SIDﾃﾞｰﾀがあれば削除する */
		Index = Mifare_WrtNgDataSearch( &(pData->Sid[0]) );			// 同一SID検索
		if( 0xffff != Index ){										// 同一SIDあり
			/* 消去するﾃﾞｰﾀの前までをﾜｰｸﾊﾞｯﾌｧにcopy */
			if( 0 != Index ){											// 先頭ﾃﾞｰﾀではない
				memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
						&MIF_WriteNgCard_ReadData.data[0], sizeof(t_MIF_CardData)*(Index) );
			}
			/* 消去するﾃﾞｰﾀを抜かして 最後までをﾜｰｸﾊﾞｯﾌｧにcopy */
			if( (MIF_WRITE_NG_BUF_COUNT - 1) != Index ){				// 最後のﾃﾞｰﾀではない
				memcpy( &MIF_WriteNgCard_ReadData.wk_data[Index], 
						&MIF_WriteNgCard_ReadData.data[Index+1], 
							sizeof(t_MIF_CardData)*((MIF_WRITE_NG_BUF_COUNT - 1) - Index) );
			}
			/* 今までFull件数だったら末尾を0ｸﾘｱする */
			if( MIF_WRITE_NG_BUF_COUNT <= MIF_WriteNgCard_ReadData.count ){	// Buffer full
				memset( &MIF_WriteNgCard_ReadData.wk_data[MIF_WRITE_NG_BUF_COUNT - 1], 0, sizeof(t_MIF_CardData) );
			}

			/* 登録件数 - 1 */
			MIF_WriteNgCard_ReadData.wk_count = MIF_WriteNgCard_ReadData.count - 1;
		}
		else{	/* 無ければ 以降ﾜｰｸｴﾘｱで作業するので ﾃﾞｰﾀをcopyする */
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
					&MIF_WriteNgCard_ReadData.data[0], sizeof(t_MIF_CardData)*MIF_WRITE_NG_BUF_COUNT );

			MIF_WriteNgCard_ReadData.wk_count = MIF_WriteNgCard_ReadData.count;
		}

		/* 1件登録処理 */
		if( MIF_WRITE_NG_BUF_COUNT <= MIF_WriteNgCard_ReadData.wk_count ){	// Buffer full
			/* 先頭１件消去 */
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
					&MIF_WriteNgCard_ReadData.wk_data[1], sizeof(t_MIF_CardData)*(MIF_WRITE_NG_BUF_COUNT - 1) );
			MIF_WriteNgCard_ReadData.wk_count = (MIF_WRITE_NG_BUF_COUNT - 1);	// 登録件数-1
		}

		/* ﾜｰｸｴﾘｱの末尾に登録 */
		memcpy( &MIF_WriteNgCard_ReadData.wk_data[MIF_WriteNgCard_ReadData.wk_count], pData, sizeof(t_MIF_CardData) );
		++MIF_WriteNgCard_ReadData.wk_count;						// 登録件数+1

		/* 実ﾃﾞｰﾀ更新(停電保障) */
		MIF_WriteNgCard_ReadData.f_Phase = 0x55;					// ﾜｰｸｴﾘｱにﾃﾞｰﾀｾｯﾄ完了
		Mifare_WrtNgDataUpdate_Exec();
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare書込み失敗時のデータ更新                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	t_MIF_WriteNgCard_ReadData のﾜｰｸｴﾘｱ内容を実ﾃﾞｰﾀｴﾘｱにcopyする		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-11-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Mifare_WrtNgDataUpdate_Exec( void )
{
	if( 0x55 != MIF_WriteNgCard_ReadData.f_Phase )					// ﾃﾞｰﾀｾｯﾄ完了ではない(copy不要)
		return;

	MIF_WriteNgCard_ReadData.count = MIF_WriteNgCard_ReadData.wk_count;
	memcpy( &MIF_WriteNgCard_ReadData.data[0], &MIF_WriteNgCard_ReadData.wk_data[0],
										 sizeof(t_MIF_CardData)*MIF_WRITE_NG_BUF_COUNT );
	MIF_WriteNgCard_ReadData.f_Phase = 0;							// copy完了
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare書込み失敗ｴﾘｱから同一SIDを持つﾃﾞｰﾀ検索                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : pSid[4]  検索するSID									   |*/
/*| RETURN VALUE : 0xffff = 対象データなし                                 |*/
/*|				   上記以外（0～）= 対象ﾃﾞｰﾀのIndex						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-11-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	Mifare_WrtNgDataSearch( uchar pSid[] )
{
	ushort	i, retus;

	retus = 0xffff;

	/* fail safe */
	if( MIF_WRITE_NG_BUF_COUNT < MIF_WriteNgCard_ReadData.count)
		MIF_WriteNgCard_ReadData.count = 0;

	for( i=0; i<MIF_WriteNgCard_ReadData.count; ++i ){				// 登録件数分
		if( 0 == memcmp( pSid, MIF_WriteNgCard_ReadData.data[i].Sid, 4 ) ){	// 同一SID発見
			retus = i;
			break;
		}
	}

	return	retus;
}

/*[]-----------------------------------------------------------------------[]*/
/*| GTﾌｫｰﾏｯﾄ設定Mifareﾁｪｯｸ処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_GT_Settei_Check_For_Mif()                            |*/
/*| PARAMETER    : gt_flg = 読み取った券がGTorAPSﾌｫｰﾏｯﾄ						|*/
/*|				   pno_syu= 基本,拡張1～3の番号								|*/
/*| RETURN VALUE : 0=設定OK,1=設定NG									    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : namioka			                                        |*/
/*| Date         : 2008/06/16                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
static short Ope_GT_Settei_Check_For_Mif( uchar gt_flg, ushort pno_syu )
{
short	ret;

	ret = 0;
	if(prm_get( COM_PRM,S_SYS,11,1,2) == 1){									// GT設定
		if( gt_flg == 1 ){														// ﾘｰﾄﾞﾃﾞｰﾀがGTﾌｫｰﾏｯﾄ
			if( !prm_get( COM_PRM,S_SYS,12,1,mgtype[pno_syu] )){				// 設定ｴﾗｰとする
				ret = 1;
			}
		}//elseの場合は、データが正常に読み出せていない為、『使用不可券です』というエラーになる
	}else{																		// APS/GT設定
		if( gt_flg == 1 ){														// ﾘｰﾄﾞﾃﾞｰﾀがGTﾌｫｰﾏｯﾄ
			if( !prm_get( COM_PRM,S_SYS,12,1,mgtype[pno_syu] )){
				ret = 2;														// 他の駐車場の券(駐車場Noｴﾗｰ)とする
			}
		}else{																	// ﾘｰﾄﾞﾃﾞｰﾀがAPSﾌｫｰﾏｯﾄ
			if( prm_get( COM_PRM,S_SYS,12,1,mgtype[pno_syu] )){
				ret = 2;														// 他の駐車場の券(駐車場Noｴﾗｰ)とする
			}
		}
	}
	return( ret );
}
