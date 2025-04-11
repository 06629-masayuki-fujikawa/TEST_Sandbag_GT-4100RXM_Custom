/*[]----------------------------------------------------------------------[]*/
/*| ﾗﾍﾞﾙﾌﾟﾘﾝﾀ関連制御                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<stdio.h>
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
#include	"prm_tbl.h"

/* A1電文構造体 */
typedef struct {
	uchar	code;			// 電文識別ｺｰﾄﾞ
	uchar	blank;			// 空き
	uchar	reserve1;		// 予備1
	uchar	print_no;		// 印字ﾃﾞｰﾀ数
	uchar	format1;		// ﾌｫｰﾏｯﾄ選択1
	uchar	format2;		// ﾌｫｰﾏｯﾄ選択2
	uchar	format3;		// ﾌｫｰﾏｯﾄ選択3
	uchar	format4;		// ﾌｫｰﾏｯﾄ選択4
	uchar	data01[23];		// ﾃﾞｰﾀ
	uchar	data02[15];
	uchar	data03[9];
	uchar	data04[9];
	uchar	data05[3];
	uchar	data06[10];
	uchar	data07[9];
	uchar	data08[23];
	uchar	data09[3];
	uchar	data10[3];
	uchar	data11[3];
	uchar	data12[7];
	uchar	data13[9];
	uchar	data14[7];
	uchar	data15[49];
	uchar	data16[25];
	uchar	data17[19];
	uchar	data18[9];
	uchar	data19[6];
	uchar	data20[9];
	uchar	data21[3];
	uchar	data22[10];
	uchar	data23[25];
	uchar	data24[3];
	uchar	data25[3];
	uchar	data26[3];
	uchar	data27[3];
	uchar	data28[23];
	uchar	data29[3];
	uchar	data30[3];
	uchar	data31[3];
	uchar	data32[27];
	uchar	data33[3];
	uchar	data34[3];
	uchar	data35[3];
	uchar	data36[27];
	uchar	data37[27];
	uchar	data38[1];
	uchar	reserve2[89];	// 予備2
} LprSndData_A1;

/* A1電文用固定送信文字列 */
static const uchar	LprString01[]	= "定期駐車券（カード用）";
static const uchar	LprString01_2[]	= "　　　　　　　　　　　";
static const uchar	LprString02[]	= "　　　／　　　";
static const uchar	LprString02_2[]	= "　　　　　　　";
static const uchar	LprString02_3[]	= "　テ　ス　ト　";
static const uchar	LprString03[]	= "No.     ";
static const uchar	LprString03_2[]	= "        ";
static const uchar	LprString04[]	= "車室番号";
static const uchar	LprString04_2[]	= "　　　　";
static const uchar	LprString05[]	= "上";
static const uchar	LprString05_3[]	= "　";
static const uchar	LprString06[]	= "A01-0256L";
static const uchar	LprString06_2[]	= "         ";
static const uchar	LprString07[]	= "有効期限";
static const uchar	LprString07_2[]	= "　　　　";
static const uchar	LprString08[]	= "年　　　月末まで　　　";
static const uchar	LprString08_2[]	= "年　　　月　　　日まで";
static const uchar	LprString08_3[]	= "　　　　　　　　　　　";
static const uchar	LprString11[]	= "  ";
static const uchar	LprString12[]	= "領収日";
static const uchar	LprString12_2[]	= "　　　";
static const uchar	LprString13[]	= "  .  .  ";
static const uchar	LprString13_2[]	= "        ";
static const uchar	LprString14[]	= "お名前";
static const uchar	LprString14_2[]	= "　　　";
static const uchar	LprString15[]	= "　　　　　　　　　　　　　　　　　　　　　　　　";
static const uchar	LprString16[]	= "　定期駐車用ステッカー　";
static const uchar	LprString17[]	= "　　　　／　　　　";
static const uchar	LprString17_2[]	= "　　テ　ス　ト　　";
static const uchar	LprString18[]	= "定期番号";
static const uchar	LprString23[]	= "有効期限　　年　　月から";
static const uchar	LprString23_2[]	= "　　年　　月　　日から　";
static const uchar	LprString27[]	= "年";
static const uchar	LprString28[]	= "　　　　　月末まで　　";
static const uchar	LprString28_2[]	= "　　月　　　　　日まで";
static const uchar	LprString32[]	= "発行日　　年　　月　　日　";
static const uchar	LprString36[]	= "　　　　　　　　　　　　　";
static const uchar	LprString32_1[]	= "発行日　　年　　月　　日※";

static const uchar	LprStrToku[]	= "＊";
static const uchar	LprStrGaku[2][5]= {"一般","学生"};
static const uchar	LprStrSyu[6][7]	= {"自転車","バイク","原付　","自二大","自二中","自二小"};


/* ﾗﾍﾞﾙﾌﾟﾘﾝﾀ送信用ﾊﾞｯﾌｧ */
static struct {
	ushort	size;
	uchar	buf[508];
} LprSndBuf;

/* static functions prototype */
static void	LprErrChk(uchar code, uchar onoff);

/*[]----------------------------------------------------------------------[]*/
/*| 送信ﾊﾞｯﾌｧ初期化                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpLpr_init( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpLpr_init(void)
{
	memset(&LprSndBuf, 0, sizeof(LprSndBuf));
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾗﾍﾞﾙﾌﾟﾘﾝﾀ制御処理                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpLpr_ctl( ret )                                        |*/
/*| PARAMETER    : msg  : ｲﾍﾞﾝﾄ                                            |*/
/*|              : data : ﾃﾞｰﾀ                                             |*/
/*| RETURN VALUE : ｲﾍﾞﾝﾄ                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	OpLpr_ctl(ushort msg, uchar *data)
{
	switch (msg){
	case IBK_LPR_SRLT_REC:							// 送信結果ﾃﾞｰﾀ受信
		if (data[0] != 0) {	/* 送信結果NG？ */
			if (LprSndBuf.size) {
			}
		}
		break;

	case IBK_LPR_ERR_REC:							// ｴﾗｰﾃﾞｰﾀ受信
		/* ｴﾗｰ情報をｼﾞｬｰﾅﾙ出力する */
		LprErrChk( data[1], data[2] );
		break;

	case IBK_LPR_B1_REC:							// 印字終了通知受信
		/* 印字終了状態 確認 */
		if (data[4] == 0x30) {
			msg = IBK_LPR_B1_REC_OK;				// ﾌﾟﾘﾝﾄ成功
		}
		else {
			msg = IBK_LPR_B1_REC_NG;				// ﾌﾟﾘﾝﾄ失敗
			LprErrChk( data[4], 1 );
		}
		break;

	default:
		break;
	}
	
	return msg;
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾗﾍﾞﾙﾌﾟﾘﾝﾀｺﾏﾝﾄﾞ送信処理(電文識別ｺｰﾄﾞ=A1h固定)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpLpr_snd( code )                                       |*/
/*| PARAMETER    : tcode : 定期個人ｺｰﾄﾞ                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpLpr_snd( Receipt_data *lpn, char test_flg )
{
	LprSndData_A1	*p;
	uchar			uc_tksyu;
	uchar			uc_toku, uc_gaku, uc_syu;
	uchar			uc_form1, uc_form2, uc_form3, uc_form4;

	OpLpr_init();													// ﾗﾍﾞﾙﾌﾟﾘﾝﾀ制御ﾓｼﾞｭｰﾙ初期化

	LprSndBuf.size = 508;

	/* 送信ﾃﾞｰﾀ作成(ﾃﾞｰﾀ部のみ) */
	p = (LprSndData_A1 *)&LprSndBuf.buf[8];

	p->code = 0xA1;													// 電文識別ｺｰﾄﾞ
	p->blank = 0;													// 空き
	p->reserve1 = 0;												// 予備1

	uc_tksyu = (uchar)(lpn->teiki.syu - 1);
	uc_toku = (uchar)prm_get( COM_PRM,S_KOU,(short)(33+10*uc_tksyu),1,3 );	// 利用種別(特割)
	uc_gaku = (uchar)prm_get( COM_PRM,S_KOU,(short)(33+10*uc_tksyu),1,2 );	// 利用種別(学割)
	uc_syu  = (uchar)prm_get( COM_PRM,S_KOU,(short)(33+10*uc_tksyu),1,1 );	// 利用種別(車種)

	uc_form1 = (uchar)prm_get( COM_PRM,S_KOU,27,1,3 );				// ﾌｫｰﾏｯﾄ選択1設定
	uc_form2 = (uchar)prm_get( COM_PRM,S_KOU,27,1,2 );				// ﾌｫｰﾏｯﾄ選択2設定
	uc_form3 = (uchar)prm_get( COM_PRM,S_KOU,27,1,1 );				// ﾌｫｰﾏｯﾄ選択3設定
	uc_form4 = (uchar)prm_get( COM_PRM,S_KOU,26,1,2 );				// ﾌｫｰﾏｯﾄ選択4設定

	p->format1 = uc_form1;											// ﾌｫｰﾏｯﾄ選択1
	p->format2 = uc_form2;											// ﾌｫｰﾏｯﾄ選択2
	p->format3 = uc_form3;											// ﾌｫｰﾏｯﾄ選択3
	p->format4 = 0;													// ﾌｫｰﾏｯﾄ選択4

	if( CPrmSS[S_KOU][23] ){										// 用紙設定2枚発行
		// 2枚発行設定時はｶｰﾄﾞ貼り付け用ﾃﾞｰﾀを作成
		p->print_no = 38;											// 印字ﾃﾞｰﾀ数
		memcpy( p->data01, LprString01, sizeof(p->data01) );		// "定期駐車券（カード用）"
		if( test_flg == 1 ){
			memcpy( p->data02, LprString02_3, sizeof(p->data02) );	// "　テ　ス　ト　"
		}else{
			memcpy( p->data02, LprString02, sizeof(p->data02) );	// "　　　／　　　"
			if( uc_toku ){											// 特割
				memcpy( p->data02, LprStrGaku[uc_gaku], 4 );		// "一般","学生"
				memcpy( &p->data02[4], LprStrToku, 2 );				// "＊"
			}else{
				memcpy( &p->data02[2], LprStrGaku[uc_gaku], 4 );	// "一般","学生"
			}
			memcpy( &p->data02[8], LprStrSyu[uc_syu], 6 );			// "自転車","バイク","原付　","自二大","自二中","自二小"
		}
		memcpy( p->data03, LprString03, sizeof(p->data03) );		// "No.     "
		intoasl( &p->data03[3], lpn->teiki.id, 5 );					// 定期券ID

		if( uc_form3 == 0 ){
			// 車室番号印字しない
			memcpy( p->data04, LprString04_2, sizeof(p->data04) );	// "　　　　"
			memcpy( p->data05, LprString05_3, sizeof(p->data05) );	// "　"
			memcpy( p->data06, LprString06_2, sizeof(p->data06) );	// "         "
		}else{
			// 車室番号印字する
			memcpy( p->data04, LprString04, sizeof(p->data04) );	// "車室番号"
			memcpy( p->data05, LprString05, sizeof(p->data05) );	// "上"			ダミー
			memcpy( p->data06, LprString06, sizeof(p->data06) );	// "A01-0256L"	ダミー
		}

		memcpy( p->data07, LprString07, sizeof(p->data07) );		// "有効期限"

		if( uc_form2 == 0 ){
			// xx月末まで
			memcpy( p->data08, LprString08, sizeof(p->data08) );	// "年　　　月末まで"
			memcpy( p->data11, LprString11, sizeof(p->data11) );	// "  "日はｽﾍﾟｰｽで埋める
		}else{
			// xx月xx日まで
			memcpy( p->data08, LprString08_2, sizeof(p->data08) );	// "年　月　日まで　"
			intoas( p->data11, (ushort)(lpn->teiki.e_day), 2 );		// 日
		}
		intoas( p->data09, (ushort)(lpn->teiki.e_year%100), 2 );	// 年
		intoas( p->data10, (ushort)(lpn->teiki.e_mon), 2 );			// 月

		memcpy( p->data12, LprString12, sizeof(p->data12) );		// "領収日"
		memcpy( p->data13, LprString13, sizeof(p->data13) );		// "  .  .  "
		intoas( &p->data13[0], (ushort)(lpn->TOutTime.Year%100), 2 );	// 年
		intoas( &p->data13[3], (ushort)(lpn->TOutTime.Mon), 2 );	// 月
		intoas( &p->data13[6], (ushort)(lpn->TOutTime.Day), 2 );	// 日
		memcpy( p->data14, LprString14, sizeof(p->data14) );		// "お名前"

		memcpy( p->data15, LprString15, sizeof(p->data15) );		// "　　　　　　　　　　　　　　　　　　　　　　　　"
		if( uc_form4 == 1 ){										// ヘッダーファイルを用いる
			// 領収証のﾍｯﾀﾞｰを流用して駐輪場名は可変に設定できるようにする。
			// 領収証のﾍｯﾀﾞｰ(Header.txt)の3、4行目をラベル用として使用する
			// ﾗﾍﾞﾙに印字できる文字数は24文字(48ﾊﾞｲﾄ)。
			// ﾍｯﾀﾞｰは1行36ﾊﾞｲﾄなので、ﾍｯﾀﾞｰの3行目の先頭から24ﾊﾞｲﾄと4行目先頭から24ﾊﾞｲﾄを使用する。
			// ﾍｯﾀﾞｰﾌｧｲﾙの3行目、4行目とも全角13文字目は全角ｽﾍﾟｰｽとする事。
			// 全て全角で設定する事。
			memcpy( p->data15, &Header_Data[2][0], 24 );
			memcpy( &p->data15[24], &Header_Data[3][0], 24 );
		}
	}else{
		// 1枚発行設定時はｶｰﾄﾞ貼り付け用ﾃﾞｰﾀはすべてスペースで埋める
		p->print_no = 37;											// 印字ﾃﾞｰﾀ数
		memcpy( p->data01, LprString01_2, sizeof(p->data01) );		// "　　　　　　　　　　　"
		memcpy( p->data02, LprString02_2, sizeof(p->data02) );		// "　　　　　　　"
		memcpy( p->data03, LprString03_2, sizeof(p->data03) );		// "        "
		memcpy( p->data04, LprString04_2, sizeof(p->data04) );		// "　　　　"
		memcpy( p->data05, LprString05_3, sizeof(p->data05) );		// "　"
		memcpy( p->data06, LprString06_2, sizeof(p->data06) );		// "         "
		memcpy( p->data07, LprString07_2, sizeof(p->data07) );		// "　　　　"
		memcpy( p->data08, LprString08_3, sizeof(p->data08) );		// "　　　　　　　　　　　"
		memcpy( p->data09, LprString11, sizeof(p->data09) );		// "  "
		memcpy( p->data10, LprString11, sizeof(p->data10) );		// "  "
		memcpy( p->data11, LprString11, sizeof(p->data11) );		// "  "
		memcpy( p->data12, LprString12_2, sizeof(p->data12) );		// "　　　"
		memcpy( p->data13, LprString13_2, sizeof(p->data13) );		// "        "
		memcpy( p->data14, LprString14_2, sizeof(p->data14) );		// "　　　"
		memcpy( p->data15, LprString15, sizeof(p->data15) );		// "　　　　　　　　　　　　　　　　　　　　　　　　"
	}

	// 車体貼り付け用ﾃﾞｰﾀ作成
	memcpy( p->data16, LprString16, sizeof(p->data16) );			// "定期駐車用ステッカー"
	memcpy( p->data17, LprString17, sizeof(p->data17) );			// "　学生＊／自転車　"

	if( test_flg == 1 ){
		memcpy( p->data17, LprString17_2, sizeof(p->data17) );		// "　　テ　ス　ト　　"
	}else{
		memcpy( p->data17, LprString17, sizeof(p->data17) );		// "　　　　／　　　　"
		if( uc_toku ){												// 特割
			memcpy( &p->data17[2], LprStrGaku[uc_gaku], 4 );		// "一般","学生"
			memcpy( &p->data17[6], LprStrToku, 2 );					// "＊"
		}else{
			memcpy( &p->data17[4], LprStrGaku[uc_gaku], 4 );		// "一般","学生"
		}
		memcpy( &p->data17[10], LprStrSyu[uc_syu], 6 );				// "自転車","バイク","原付　","自二大","自二中","自二小"
	}

	memcpy( p->data18, LprString18, sizeof(p->data18) );			// "定期番号"
	intoasl( p->data19, lpn->teiki.id, 5 );							// 定期券ID

	if( uc_form3 == 0 ){
		// 車室番号印字しない
		memcpy( p->data20, LprString04_2, sizeof(p->data20) );		// "　　　　"
		memcpy( p->data21, LprString05_3, sizeof(p->data21) );		// "　"
		memcpy( p->data22, LprString06_2, sizeof(p->data22) );		// "         "
	}else{
		// 車室番号印字する
		memcpy( p->data20, LprString04, sizeof(p->data20) );		// "車室番号"
		memcpy( p->data21, LprString05, sizeof(p->data21) );		// "上"			ダミー
		memcpy( p->data22, LprString06, sizeof(p->data22) );		// "A01-0256L"	ダミー
	}

	if( uc_form1 == 0 ){
		// 有効期限xx年xx月から
		memcpy( p->data23, LprString23, sizeof(p->data23) );		// "有効期限　　年　　月から"
		memcpy( p->data26, LprString11, sizeof(p->data26) );		// "  "日はｽﾍﾟｰｽで埋める
	}else{
		// xx年xx月xx日から
		memcpy( p->data23, LprString23_2, sizeof(p->data23) );		// "　　年　　月　　日から　"
		intoas( p->data26, (ushort)(lpn->teiki.s_day), 2 );			// 日
	}
	intoas( p->data24, (ushort)(lpn->teiki.s_year%100), 2 );		// 年
	intoas( p->data25, (ushort)(lpn->teiki.s_mon), 2 );				// 月

	memcpy( p->data27, LprString27, sizeof(p->data27) );			// "年"
	if( uc_form2 == 0 ){
		// xx月末まで
		memcpy( p->data28, LprString28, sizeof(p->data28) );		// "　　　　　月末まで　　"
		memcpy( p->data31, LprString11, sizeof(p->data31) );		// "  "日はｽﾍﾟｰｽで埋める
	}else{
		// xx月xx日まで
		memcpy( p->data28, LprString28_2, sizeof(p->data28) );		// "月　　　　　日まで　　"
		intoas( p->data31, (ushort)(lpn->teiki.e_day), 2 );			// 日
	}
	intoas( p->data29, (ushort)(lpn->teiki.e_year%100), 2 );		// 年
	intoas( p->data30, (ushort)(lpn->teiki.e_mon), 2 );				// 月

	if( test_flg == 2 ){
		memcpy( p->data32, LprString32_1, sizeof(p->data32) );			// "発行日　　年　　月　　日※"
	}else{
		memcpy( p->data32, LprString32, sizeof(p->data32) );			// "発行日　　年　　月　　日"
	}
	intoas( p->data33, (ushort)(lpn->TOutTime.Year%100), 2 );		// 年
	intoas( p->data34, (ushort)(lpn->TOutTime.Mon), 2 );			// 月
	intoas( p->data35, (ushort)(lpn->TOutTime.Day), 2 );			// 日

	memcpy( p->data36, LprString36, sizeof(p->data36) );			// "　　　　　　　　　　　　　"
	memcpy( p->data37, LprString36, sizeof(p->data37) );			// "　　　　　　　　　　　　　"
	if( uc_form4 == 1 ){											// ヘッダーファイルを用いる?
		// 領収証のﾍｯﾀﾞｰを流用して駐輪場名は可変に設定できるようにする。
		// 領収証のﾍｯﾀﾞｰ(Header.txt)の3、4行目をラベル用として使用する
		// ﾗﾍﾞﾙに印字できる文字数は13文字(26ﾊﾞｲﾄ)×2行。
		// ﾍｯﾀﾞｰは1行36ﾊﾞｲﾄなので、ﾍｯﾀﾞｰの3行目の先頭から24ﾊﾞｲﾄと4行目先頭から24ﾊﾞｲﾄを使用する。
		// ﾍｯﾀﾞｰﾌｧｲﾙの3行目、4行目とも全角13文字目は全角ｽﾍﾟｰｽとする事。
		// 全て全角で設定する事。
		memcpy( p->data36, &Header_Data[2][0], 24 );
		memcpy( p->data37, &Header_Data[3][0], 24 );
	}

	/* 送信 */
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾗﾍﾞﾙ関連ｴﾗｰ登録処理                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LprErrChk( code, onoff )                                |*/
/*| PARAMETER    : code   : ｴﾗｰ/ｱﾗｰﾑｺｰﾄﾞ                                   |*/
/*|              : onoff  : 0=解除 1=発生                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-03-17                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static void	LprErrChk(uchar code, uchar onoff)
{
	uchar	err_arm;

	switch (code) {
	/* ｴﾗｰｺｰﾄﾞ */
	case 0x01:		/* IBKプリンタモジュール - プリンタ間通信タイムアウト */
		code = 1;
		err_arm = 1;
		break;
	case 0x07:		/* ラベルプリンタ　オンライン／オフライン状態 */
	case 0x34:		/* オフライン */
		code = 20;
		err_arm = 1;
		break;
	case 0x61:		/* バッファオーバー */
		code = 14;
		err_arm = 1;
		break;
	case 0x62:		/* ヘッドオープン */
		code = 11;
		err_arm = 1;
		break;
	case 0x65:		/* メディアエラー */
		code = 15;
		err_arm = 1;
		break;
	case 0x66:		/* センサエラー */
		code = 13;
		err_arm = 1;
		break;
	case 0x67:		/* ヘッドエラー */
		code = 12;
		err_arm = 1;
		break;
	case 0x6a:		/* カッタエラー */
		code = 10;
		err_arm = 1;
		break;
	case 0x6b:		/* その他のエラー */
		code = 16;
		err_arm = 1;
		break;

	/* ｱﾗｰﾑｺｰﾄﾞ */
	case 0x63:		/* ペーパーエンド */
		code = 46;
		err_arm = 2;
		break;
	case 0x64:		/* リボンエンド */
		code = 47;
		err_arm = 2;
		break;
	case 0x31:		/* リボンニアエンド */
		code = 49;
		err_arm = 2;
		break;
	default:
		err_arm = 0;
		break;
	}

	if( err_arm == 1 ){
		err_chk( ERRMDL_LABEL, (char)code, (char)onoff, 0, 0 );	// ﾗﾍﾞﾙｴﾗｰ登録
	}
	else if( err_arm == 2 ){
		alm_chk( ALMMDL_SUB, (char)code, (char)onoff );			// ﾗﾍﾞﾙｱﾗｰﾑ登録
	}
}
