/**********************************************************************************************************************/
/*                                                                                                                    */
/*  関数名称          ：料金計算要求作成（ＶＬ）    記述                                                              */
/*                                                ：--------------------------------------------------------------：  */
/*  関数シンボル      ：ec09()                    ：  入力されたＶＬデータに対応する料金計算を行ってもらう為の要求：  */
/*                                                ：要求テーブルを作成する。                                      ：  */
/*                                                ：--------------------------------------------------------------：  */
/*                                                ：入力パラメータ              出力パラメータ                    ：  */
/*                                                ：  	無し			            無し                          ：  */
/*                                                ：--------------------------------------------------------------：  */
/*  変更履歴          ：中山 (04/07/25)     	  ：・精算中止後のサービス券対応の移植 	  						  ：  */
/*                    ：				          ：  (FCR-P30033 TF4800N DH917004)								  ：  */
/*                                                ：--------------------------------------------------------------：  */
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"system.h"													/*                                            */
#include	"mem_def.h"													/*                                            */
#include	"rkn_def.h"													/*  全デファイン統括                          */
#include	"rkn_cal.h"													/*  料金関連データ                            */
#include	"rkn_fun.h"													/*  全サブルーチン宣言                        */
#include	"tbl_rkn.h"													/*                                            */
#include	"prm_tbl.h"													/*                                            */
#include	"ope_def.h"													/*                                            */
#include	"flp_def.h"
#include	"LKmain.h"													/*											  */
																		/*                                            */
char	ec09( void )													/*                                            */
{																		/*                                            */
	char	sdat;														/*  設定データ                                */
	char	no;															/*                                            */
	char	ans;														/*                                            */
	char	i;															/*                                            */
	char	mai;														/*                                            */
	short	sv_tim;														/*  割引時間                                  */
	short	tim_t[5];													/**                                           */
	short	pno;														/** 設定駐車場ＮＯ．                          */
	short	pnk;														/** 設定拡張駐車場ＮＯ．                      */
	short	pnk2;														/** 設定拡張2駐車場ＮＯ．                     */
	short	pnk3;														/** 設定拡張3駐車場ＮＯ．                     */
	long	l_dat;														/*                                            */
	struct	REQ_RKN		*rkn_p;											/*                                            */
	struct	REQ_CRD		*rkn_c;											/*                                            */
	struct	VL_KAS		*kas;											/** 回数券データ                              */
	struct	VL_SVS		*svs;											/** サービス券データ                          */
	struct	REQ_TKC		*rkn_t;											/*                                            */
// MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
	ushort	mise_num;													/*                                            */
	ushort	mise_use;													/*                                            */
// MH810100(E) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
																		/*                                            */
/**********************************************************************************************************************/
																		/*                                            */
	ans = OK;															/*                                            */
	memset( &req_rkn.param,0x00,21 );									/*  料金要求テーブルクリア                    */
	rkn_p = &req_rkn;												 	/*                                            */
// MH810100(S) 2020/07/27 コードレビュー指摘事項対応
	memset( &req_crd,0x00,sizeof(req_crd) );							/*  テーブルクリア                    */
// MH810100(E) 2020/07/27 コードレビュー指摘事項対応
	req_crd_cnt = 0;													/*  駐車券読取り時  券ﾃﾞｰﾀ要求件数ｸﾘｱ         */
	req_tkc_cnt = 0;													/*  定期券中止精算要求件数ｸﾘｱ                 */
	rkn_p->syubt = syashu;												/*  車種セット                                */
																		/*                                            */
	pno = (short)0;														/** 基本駐車場                                */
	pnk = (short)1;														/** 拡張1駐車場                               */
	pnk2 = (short)2;													/** 設定拡張2駐車場                           */
	pnk3 = (short)3;													/** 設定拡張3駐車場                           */
																		/*                                            */
	switch( vl_now )													/*  パラメータＶＬデータにより分岐            */
	{																	/*                                            */
		case V_CHM:														/*  駐車券（精算前）                          */
		case V_CHG:														/*  駐車券（精算後）                          */
		case V_CHS:														/*  駐車券（中止券）                          */
		case V_CHU:														/*  掛売券                                    */
																		/*                                            */
			rkn_p->param = RY_TSA;										/*  パラメータセット                          */
																		/*                                            */
			rysyasu = syashu;											/*  車種セット                                */
			sv_tim = Prm_RateDiscTime[syashu-1];						/*                                            */
			rkn_p->data[0] = sv_tim;									/*  種別時間割引セット                        */
			ans = OK;													/*                                            */
																		/*                                            */
			mai = (uchar)prm_get(COM_PRM, S_DIS, 1, 2, 1);
			if( vl_tik.wari ){											/*  割引金額(中止,修正用)                     */
				rkn_c = &req_crd[req_crd_cnt];							/*  駐車券読み取り時券データ要求格納バッファ  */
				rkn_c->param = RY_RWR;									/*  パラメータセット                          */
				rkn_c->data[0] = vl_tik.wari;							/*  割引金額セット                            */
				req_crd_cnt++;											/*  駐車券読み取り時券データ要求件数ＵＰ      */
				PayData.PRTwari.BIT.RWARI = 1;							/* 前回料金割引フラグ                         */
			}															/*                                            */
			if( vl_tik.time ){											/*  割引時間数(中止,修正用)                   */
				rkn_c = &req_crd[req_crd_cnt];							/*  駐車券読み取り時券データ要求格納バッファ  */
				rkn_c->param = RY_TWR;									/*  パラメータセット                          */
				rkn_c->data[0] = vl_tik.time;							/*  割引時間数セット                          */
				req_crd_cnt++;											/*  駐車券読み取り時券データ要求件数ＵＰ      */
				PayData.PRTwari.BIT.TWARI = 1;							/* 前回時間割引フラグ                         */
			}															/*                                            */
																		/*                                            */
			if( vl_tik.kry != 0 )										/*  料金差引割り引き有り					  */
			{															/*                                            */
				rkn_c = &req_crd[req_crd_cnt];							/*  駐車券読み取り時券データ要求格納バッファ  */
				rkn_c->param = RY_KGK;									/*  パラメータセット（料金差引割り引き）      */
				rkn_c->data[0] = vl_tik.kry;							/*  金額セット                                */
				rkn_c->data[1]= 0L;										/*  とりあえず回数券使用可=0とする            */
					if( CPrmSS[S_PRP][1] != 2 )							/*  設定がPｶｰﾄﾞ使用可 =1 or 両方なし          */
					rkn_c->data[1]= 1L;									/*  Pｶｰﾄﾞ使用可に変更する                     */
				req_crd_cnt++;											/*  駐車券読み取り時券データ要求件数ＵＰ      */
			}															/*                                            */
																		/*											  */
			if(Flap_Sub_Flg == 1){										/* 再精算時(精算中止機能強化)                 */
				for(i = 0 ; i < 15 ; i++){								/* ｻｰﾋﾞｽ券                                    */
					if( FLAPDT_SUB[Flap_Sub_Num].sev_tik[i] != 0){
						rkn_c 		 	= &req_crd[req_crd_cnt];		/*  駐車券読み取り時券データ要求格納バッファ  */
						rkn_c->param 	= RY_CSK;						/*  パラメータセット（精算中止サービス券）    */
						l_dat 		 	= i+1;							/*  種類セット                                */
						rkn_c->data[1] 	= FLAPDT_SUB[Flap_Sub_Num].sev_tik[i];	/*	枚数セット                        */
						rkn_c->data[0] 	= l_dat << 16;					/*	種類と掛け売りＮｏ．セット                */
						req_crd_cnt		++;								/*	駐車券読み取り時券データ要求件数ＵＰ      */

						if( prm_get( COM_PRM, S_DIS, 5, 1, 2 ) == 0 ){
							card_use2[i] = FLAPDT_SUB[Flap_Sub_Num].sev_tik[i];	/*ｻｰﾋﾞｽ券毎使用枚数ﾃｰﾌﾞﾙ                  */
						}
					}
				}
				for(i = 0 ; i < 5 ; i++){								/* 掛売券                                     */
					if(FLAPDT_SUB[Flap_Sub_Num].kake_data[i].mise_no != 0){
						rkn_c 		 	= &req_crd[req_crd_cnt];		/*  駐車券読み取り時券データ要求格納バッファ  */
						rkn_c->param 	= RY_CMI;						/*  パラメータセット（精算中止掛売券）        */
						rkn_c->data[1] 	= FLAPDT_SUB[Flap_Sub_Num].kake_data[i].maisuu;	/*	種類と掛け売りＮｏ．セット*/
						rkn_c->data[0] 	= FLAPDT_SUB[Flap_Sub_Num].kake_data[i].mise_no;	/*	店Noセット            */
						req_crd_cnt		++;								/*	駐車券読み取り時券データ要求件数ＵＰ      */
					}
				}
				PayData_Sub.pay_ryo = FLAPDT_SUB[Flap_Sub_Num].ppc_chusi_ryo;/* 回数券orプリペイドカード              */
				PayData.ppc_chusi = PayData_Sub.pay_ryo;				/* 回数券orプリペイドカード中止バッファ       */
				if(PayData_Sub.pay_ryo){
					Pay_Flg = 1;		//プリペイド、回数券使用ﾌﾗｸﾞ
				}
			}
// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
			l_dat = ryo_buf.zenkai;
			if (l_dat > 999990){
				l_dat = 999990;
			}

																	/*                                            */
			if (l_dat > 0){
				rkn_c = &req_crd[req_crd_cnt];						/*  駐車券読み取り時券データ要求格納バッファ  */
				rkn_c->param = RY_RWR;								/*  パラメータセット（）      */
				rkn_c->data[0] = l_dat;								/*  金額セット                                */
				req_crd_cnt++;										/*  駐車券読み取り時券データ要求件数ＵＰ      */
				PayData.PRTwari.BIT.RWARI = 1;							/* 前回料金割引フラグ                         */
			}
// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
			prk_kind = 1;												/*  基本セット                                */
		break;															/*                                            */
// MH322914 (s) kasiyama 2016/07/15 AI-V対応

		// 紛失券
		case V_FUN:
			rkn_p->param = RY_FKN;			// パラメータセット

			if( OPECTL.CalStartTrigger == (uchar)CAL_TRIGGER_REMOTECALC_FEE )
			{
// 仕様変更(S) K.Onodera 2016/11/07 遠隔精算フォーマット変更対応
//				rysyasu = LockInfo[ryo_buf.pkiti-1].ryo_syu;
//				syashu = rysyasu;
// MH810100(S) K.Onodera 2020/03/04 車番チケットレス(#3985 遠隔からの後日精算時の領収証に料金種別が印字されない不具合修正)
//				if( !g_PipCtrl.stRemoteFee.Type ){
//					rysyasu = LockInfo[ryo_buf.pkiti-1].ryo_syu;
//					syashu = rysyasu;		// フラップに設定された種別
//				}else{
// MH810100(E) K.Onodera 2020/03/04 車番チケットレス(#3985 遠隔からの後日精算時の領収証に料金種別が印字されない不具合修正)
					// 要求に種別がセットされている？
					if( g_PipCtrl.stRemoteFee.RyoSyu >= 1 && g_PipCtrl.stRemoteFee.RyoSyu <= 12 ){
						rysyasu = g_PipCtrl.stRemoteFee.RyoSyu;
					}else{
						rysyasu = prm_get( COM_PRM, S_CEN, 40, 2, 1 );
						if( !rysyasu ){
							rysyasu = syashu = 1;	// A種固定
						}
						g_PipCtrl.stRemoteFee.RyoSyu = rysyasu;
					}
// MH810100(S) K.Onodera 2020/03/04 車番チケットレス(#3985 遠隔からの後日精算時の領収証に料金種別が印字されない不具合修正)
//				}
// MH810100(E) K.Onodera 2020/03/04 車番チケットレス(#3985 遠隔からの後日精算時の領収証に料金種別が印字されない不具合修正)
// 仕様変更(E) K.Onodera 2016/11/07 遠隔精算フォーマット変更対応
				cr_tkpk_flg = 2;
			}
			prk_kind = 1;					// 基本セット
			break;
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
																		/*                                            */
		case V_SAK:														/*  サービス券                                */
			svs = &vl_svs;												/**                                           */
			if( svs->pkno == pno )			prk_kind = 1	;			/** 駐車場Ｎｏ．が一致なら                    */
			else if( svs->pkno == pnk )		prk_kind = 2    ;			/** 拡張駐車場Ｎｏ．が一致なら                */
			else if( svs->pkno == pnk2 )	prk_kind = 3    ;			/** 拡張2駐車場Ｎｏ．が一致なら               */
			else if( svs->pkno == pnk3 )	prk_kind = 4    ;			/** 拡張3駐車場Ｎｏ．が一致なら               */
			else							prk_kind = 0 	;			/**                                           */
																		/*                                            */
// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
			mise_num = (ushort)( vl_svs.mno & 0x7fff );					/* 店(掛売先)№のみ取出し                     */
			mise_use = (ushort)vl_svs.mno;								/* long型へのキャストを考慮しushort型へ       */
// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// MH810100(S) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
			l_dat = 0;
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
//			if(vl_svs.sno > 50){
//				// +50して店割引の割引種類を格納してあるので分離する（サービス券の場合は、1～26）
//				l_dat = vl_svs.sno -50;
			if ( vl_svs.sno > 30 ) {
				// 多店舗割引種別と店割引種類は30以上加算して格納しているため分離する
				l_dat = vl_svs.sno - 30;
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
				vl_svs.sno = 0;
			}
// MH810100(E) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
			if( vl_svs.sno == 0 )										/*  店Ｎｏ．割引要求                          */
			{															/*                                            */
// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
//				sdat = (char)CPrmSS[S_STO][3+((vl_svs.mno-1)*3)];		/*  切換先の車種を取得                        */
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
//				sdat = (char)CPrmSS[S_STO][3+((mise_num-1)*3)];			/*  切換先の車種を取得                        */
				if ( mise_num > MISE_NO_CNT ) {
					// 多店舗割引設定から取得
					sdat = (char)prm_get(COM_PRM, S_TAT, (mise_num - 69), 2, 1);
				} else {
					// 店割引設定から取得
					sdat = (char)prm_get(COM_PRM, S_STO, (mise_num * 3), 2, 1);
				}
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
				if( sdat != 0 ) 										/*  車種切り替え有り？                        */
				{														/*                                            */
					rysyasu = sdat;										/*  車種セット                                */
					syashu = rysyasu;									/*  車種セット                                */
																		/*                                            */
					rkn_p->syubt = syashu;								/*  車種セット                                */
					rkn_p->param = RY_KCH;								/*  パラメータセット（車種切換）              */
					sv_tim = Prm_RateDiscTime[syashu-1];				/*  種別時間割引                              */
					rkn_p->data[0] = sv_tim;							/*  種別時間割引セット                        */
// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
//					rkn_p->data[1] = ( (long)3 << 16 | (long)vl_svs.mno);/*  券データと店Ｎｏ．セット                 */
					rkn_p->data[1] = ( (long)3 << 16 | (long)mise_num);	/*  券データと店Ｎｏ．セット                 */
// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
																		/*                                            */
					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(sdat-1)),2,5 ) )/*  車種切換先料金種別の設定あり          */
					{													/*                                            */
						rkn_c = &req_crd[req_crd_cnt];					/*  駐車券読み取り時券データ要求格納バッファ  */
						rkn_c->param = RY_KAK;							/*  掛け売り券セット                          */
						if( prk_kind == 2 )								/*  拡張駐車Ｎｏ．の場合 パラメータセット     */
							rkn_c->param = RY_KAK_K;					/*                                            */
						else if( prk_kind == 3 )						/*  拡張駐車2Ｎｏ．の場合 パラメータセット    */
							rkn_c->param = RY_KAK_K2;					/*                                            */
						else if( prk_kind == 4 )						/*  拡張駐車3Ｎｏ．の場合 パラメータセット    */
							rkn_c->param = RY_KAK_K3;					/*                                            */
// MH810100(S) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
//// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
////						rkn_c->data[0] = vl_svs.mno;					/*  店Ｎｏ．セット                            */
//						rkn_c->data[0] = (long)mise_use;				/*  使用済みフラグ＋店№セット                */
//// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
//						rkn_c->data[0] = l_dat << 16;					/*  割引種類セット                           */
						if ( l_dat > 20 ) {
							// 店割引の割引種類は50加算して格納しているため更に20減算して分離する
							rkn_c->data[0] = ((l_dat - 20) << 16) & 0x00ff0000;		/*  店割引種類セット              */
						} else if ( l_dat > 0 ) {
							rkn_c->data[0] = (l_dat << 24) & 0xff000000;			/*  多店舗割引種別セット          */
						}
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
						rkn_c->data[0] |= (long)mise_use;				/*  使用済みフラグ＋店№セット           		*/
// MH810100(E) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
						rkn_c->data[1] = 1;								/*  枚数セット                                */
						req_crd_cnt++;									/*  駐車券読み取り時券データ要求件数ＵＰ      */
					}													/*                                            */
				}														/*                                            */
				else													/*                                            */
				{														/*                                            */
					rkn_p->param = RY_KAK;								/*  掛け売り券セット                          */
					if( prk_kind == 2 )									/*  拡張駐車Ｎｏ．の場合 パラメータセット     */
						rkn_p->param = RY_KAK_K;						/*                                            */
					else if( prk_kind == 3 )							/*  拡張駐車2Ｎｏ．の場合 パラメータセット    */
						rkn_p->param = RY_KAK_K2;						/*                                            */
					else if( prk_kind == 4 )							/*  拡張駐車3Ｎｏ．の場合 パラメータセット    */
						rkn_p->param = RY_KAK_K3;						/*                                            */
// MH810100(S) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
//// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
////					rkn_p->data[0] = vl_svs.mno;						/*  店Ｎｏ．セット                            */
//					rkn_p->data[0] = (long)mise_use;					/*  使用済みフラグ＋店№セット                */
//// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
//					rkn_p->data[0] = l_dat << 16;						/*  割引種類セット                          */
					if ( l_dat > 20 ) {
						// 店割引の割引種類は50加算して格納しているため更に20減算して分離する
						rkn_p->data[0] = ((l_dat - 20) << 16) & 0x00ff0000;		/*  店割引種類セット                  */
					} else if ( l_dat > 0 ) {
						rkn_p->data[0] = (l_dat << 24) & 0xff000000;			/*  多店舗割引種別セット              */
					}
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算要求の変更)
					rkn_p->data[0] |= (long)mise_use;					/*  使用済みフラグ＋店№セット              */
// MH810100(E) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
					rkn_p->data[1] = 1;									/*  枚数セット                                */
				}														/*                                            */
			}															/*                                            */
			else														/*  サービス券                                */
			{															/*                                            */
				sdat = (char)CPrmSS[S_SER][3+((vl_svs.sno-1)*3)];		/*  切換先の車種を取得                        */
				if( sdat != 0 ) 										/*  車種切り替え有り？                        */
				{														/*                                            */
					rysyasu = sdat;										/*  車種セット                                */
					syashu = rysyasu;									/*  車種セット                                */
																		/*                                            */
					rkn_p->syubt = syashu;								/*  車種セット                                */
					rkn_p->param = RY_KCH;								/*  パラメータセット（車種切換）              */
					sv_tim = Prm_RateDiscTime[syashu-1];				/*  種別時間割引                              */
					rkn_p->data[0] = sv_tim;							/*  種別時間割引セット                        */
					rkn_p->data[1] = ( (long)1 << 16 | (long)vl_svs.sno);/*  券データとサービス券の種類セット         */
																		/*                                            */
					if( prm_get( COM_PRM,S_SHA,(short)(1+6*(sdat-1)),2,5 ) )/*  車種切換先料金種別の設定あり          */
					{													/*                                            */
						rkn_c = &req_crd[req_crd_cnt];					/*  駐車券読み取り時券データ要求格納バッファ  */
						rkn_c->param = RY_SKC;							/*  パラメータセット                          */
						if( prk_kind == 2 )								/*  拡張駐車Ｎｏ．の場合 パラメータセット     */
							rkn_c->param = RY_SKC_K;					/*                                            */
						else if( prk_kind == 3 )						/*  拡張駐車2Ｎｏ．の場合 パラメータセット    */
							rkn_c->param = RY_SKC_K2;					/*                                            */
						else if( prk_kind == 4 )						/*  拡張駐車3Ｎｏ．の場合 パラメータセット    */
							rkn_c->param = RY_SKC_K3;					/*                                            */
						l_dat = vl_svs.sno;								/*  種類セット                                */
						rkn_c->data[1] = 1;								/*  枚数セット                                */
// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
//						rkn_c->data[0] =								/*  種類と掛け売りＮｏ．セット                */
//								( (l_dat << 16) | (long)vl_svs.mno  );	/*                                            */
						rkn_c->data[0] = l_dat << 16;					/*  種類セット                                */
						rkn_c->data[0] |= (long)mise_use;				/*  使用済みフラグ＋掛売先№セット            */
// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
						req_crd_cnt++;									/*  駐車券読み取り時券データ要求件数ＵＰ      */
					}													/*                                            */
				}														/*                                            */
				else													/*                                            */
				{														/*                                            */
					rkn_p->param = RY_SKC;								/*  パラメータセット                          */
					if( prk_kind == 2 )									/*  拡張駐車Ｎｏ．の場合 パラメータセット     */
						rkn_p->param = RY_SKC_K;						/*                                            */
					else if( prk_kind == 3 )							/*  拡張駐車2Ｎｏ．の場合 パラメータセット    */
						rkn_p->param = RY_SKC_K2;						/*                                            */
					else if( prk_kind == 4 )							/*  拡張駐車3Ｎｏ．の場合 パラメータセット    */
						rkn_p->param = RY_SKC_K3;						/*                                            */
					l_dat = vl_svs.sno;									/*  種類セット                                */
					rkn_p->data[1] = 1;									/*  枚数セット                                */
// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
//					rkn_p->data[0] =									/*  種類と掛け売りＮｏ．セット                */
//							( (l_dat << 16) | (long)vl_svs.mno  );		/*                                            */
					rkn_p->data[0] = l_dat << 16;						/*  種類セット                                */
					rkn_p->data[0] |= (long)mise_use;					/*  使用済みフラグ＋掛売先№セット            */
// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
				}														/*                                            */
			}															/*                                            */
		break;															/*                                            */
																		/*                                            */
		case V_SYU:														/*  サービス券／定期券（種別切換）            */
			ans = ec191( V_SYU,0,syashu,0 );							/*  車種切換ＣＨＥＣＫ                        */
		break;															/*                                            */
																		/*                                            */
		case V_KAM:														/*  回数券（無料）                            */
		case V_KAG:														/*  回数券（割引）                            */
			kas = &vl_kas;												/**                                           */
			if( kas->pkno == pno )			prk_kind = 1;				/** 駐車場Ｎｏ．が一致なら      基本セット    */
			else if( kas->pkno == pnk )		prk_kind = 2;				/** 拡張駐車場Ｎｏ．が一致なら  拡張セット    */
			else if( kas->pkno == pnk2 )	prk_kind = 3;				/** 拡張2駐車場Ｎｏ．が一致なら               */
			else if( kas->pkno == pnk3 )	prk_kind = 4;				/** 拡張3駐車場Ｎｏ．が一致なら               */
			else							prk_kind = 0 ;				/** 駐車場NO.が一致しないなら                 */
																		/*                                            */
			rkn_p->param = RY_FRE;										/*  パラメータセット                          */
			if( prk_kind == 2 )				rkn_p->param = RY_FRE_K;	/*  拡張駐車Ｎｏ．の場合  パラメータセット    */
			else if( prk_kind == 3 )		rkn_p->param = RY_FRE_K2;	/*  拡張駐車2Ｎｏ．の場合 パラメータセット    */
			else if( prk_kind == 4 )		rkn_p->param = RY_FRE_K3;	/*  拡張駐車3Ｎｏ．の場合 パラメータセット    */
			rkn_p->data[0] = vl_kas.nno;								/*  残り回数セット                            */
			rkn_p->data[1] = vl_kas.tnk;								/*  １回数単価セット                          */
			if( vl_now == V_KAM)			rkn_p->data[1] 	= 0xff;		/*  回数券（無料）  １回数単価セット（ＦＦ）  */
		break;															/*                                            */
																		/*                                            */
		case V_TST:														/*  定期券（駐車券併用なし）                  */
		case V_TSC:														/*  定期券（    〃    有り）                  */
			if( vl_now == V_TST)										/** 定期券（駐車券併用なし）                  */
			{															/**                                           */
				// ロック装置タイプのため、駐車券併用なしは機能しない。使用する時に作りこむ事。MH544401追記
				if( cr_dat_n == RID_APS )								/*                                            */
				{														/**                                           */
					for( i = 0; i < 4; i ++ ) 							/**                                           */
					{													/**                                           */
						tim_t[i] = (short)tsn_tki.sttim[i];				/**                                           */
					}													/**                                           */
					cm13( 1,tim_t );									/** 年月日範囲チェック                        */
					if( tsn_tki.status != 0 )							/**                                           */
					{													/**                                           */
						org_in.year	= tim_t[4];							/** 入庫年を現在年とする                      */
						org_in.mon	= (char)tim_t[0];					/** ステータス時刻（月）                      */
						org_in.day	= (char)tim_t[1];					/** ステータス時刻（日）                      */
						org_in.hour	= (char)tim_t[2];					/** ステータス時刻（時）                      */
						org_in.min	= (char)tim_t[3];					/** ステータス時刻（分）                      */
						org_in.week	=									/** 入庫曜日セット                            */
							(char)youbiget( tim_t[4],tim_t[0],tim_t[1]);/**                                           */
					}													/**                                           */
					else 												/** '93.8.3 tazou start                       */
					{													/**                                           */
						org_in.year = hzuk.y;			 				/** 入庫年を現在年とする                      */
						org_in.mon 	= hzuk.m;      		 				/** 入庫年を現在時刻（月）                    */
						org_in.day 	= hzuk.d;      		 				/** 入庫年を現在時刻（日）                    */
						org_in.week	= hzuk.w;			 				/** 入庫曜日セット                            */
						org_in.hour = jikn.t;      		 				/** 入庫年を現在時刻（時）                    */
						org_in.min 	= jikn.m;       					/** 入庫年を現在時刻（分）                    */
					}													/**                                           */
				}														/**                                           */
			}															/**                                           */
			if( ryo_buf.pass_zero == 1 ){								/** 定期券強制０円精算要求あり                */
				memcpy( &org_in, &org_ot, sizeof( struct CAR_TIM ) );	/** 入出庫同時刻とする                        */
			}															/**                                           */
			if( cr_dat_n == RID_APS )									/** ＡＰＳ定期の場合                          */
			{															/**                                           */
				if( tsn_tki.pkno == pno )				prk_kind = 1;	/** 基本駐車場№が一致なら  基本セット        */
				else if( tsn_tki.pkno == pnk )			prk_kind = 2;	/** 拡張駐車場№が一致なら  拡張セット        */
				else if( tsn_tki.pkno == pnk2 )			prk_kind = 3;	/** 拡張2駐車場№が一致なら                   */
				else if( tsn_tki.pkno == pnk3 )			prk_kind = 4;	/** 拡張3駐車場№が一致なら                   */
				else									prk_kind = 0;	/**                                           */
			}															/**                                           */
			if( cr_dat_n == RID_JIS )					prk_kind = 1;	/** ＪＩＳ定期の場合                          */
			rkn_p->param = RY_PKC;										/*  パラメータセット                          */
			if( prk_kind == 2 )			rkn_p->param = RY_PKC_K;		/*  拡張駐車Ｎｏ．の場合  パラメータセット    */
			else if( prk_kind == 3 )	rkn_p->param = RY_PKC_K2;		/*  拡張駐車2Ｎｏ．の場合 パラメータセット    */
			else if( prk_kind == 4 )	rkn_p->param = RY_PKC_K3;		/*  拡張駐車3Ｎｏ．の場合 パラメータセット    */
			rkn_p->data[0] = tsn_tki.kind;								/*  定期種類                                  */
																		/*                                            */
 			cr_tkpk_flg = 1  ;											/*  定期券使用情報     						  */
			if( tki_pk_use <= 2 )		tki_pk_use ++ 	;				/*  定期/駐車券使用   	現在+1		      	  */
																		/*                                            */
																		/*                                            */
																		/*                                            */
			if( tsn_tki.status == 1  	&& 								/*  ｽﾃｰﾀｽは出庫中                             */
				tkcyu_ichi 			 	&& 								/*  中止定期ﾃﾞｰﾀ有り						  */
				vl_now 		   == V_TST 	)							/*  駐車券併用なし                            */
			{															/*                                            */
				// ロック装置タイプのため、駐車券併用なしは機能しない。使用する時に作りこむ事。MH544401追記
				if( (no = (char)tki_cyusi.dt[tkcyu_ichi-1].mno) != 0 )	/*  定期券読み取り時  店データ有り            */
				{														/*                                            */
					sdat = (char)CPrmSS[S_STO][3+((vl_svs.mno-1)*3)];	/* 切換先の車種を取得						*/
																		/*                                            */
					if( sdat != 0 )										/*  車種切り替え有り？                        */
					{													/*                                            */
						ans = ec191( V_CHG,no,sdat,0 );					/*  車種切換ＣＨＥＣＫ                        */
					}													/*                                            */
					rkn_t 		 	= &req_tkc[req_tkc_cnt];			/*  定期中止ﾃﾞｰﾀ要求格納ﾊﾞｯﾌｧ  			   	  */
					rkn_t->param 	= RY_CMI;							/*  パラメータセット（店Ｎｏ．割り引き）      */
					rkn_t->data[0] 	= no;								/*  店Ｎｏ．セット                            */
					rkn_t->data[1] 	= 1;								/*  枚数セット                                */
					req_tkc_cnt		++;									/*  定期券中止精算要求納件数 UP  	    	  */
				}														/*                                            */
																		/*                                            */
				mai = (uchar)prm_get(COM_PRM, S_DIS, 1, 2, 1);
				for( i = 0; i < 3; i++ )								/*  サービス券種分                            */
				{														/*                                            */
					if( (no = tki_cyusi.dt[tkcyu_ichi-1].sa[i]) != 0 )	/*  Ａ～Ｃサービス券の枚数有り？              */
					{													/*                                            */
						if( mai > card_use[USE_SVC] )					/*  サービス券使用枚数の限度チェック          */
						{												/*                                            */
							if( mai <= (no + card_use[USE_SVC]) )		/*  サービス券使用枚数が限度を越えている      */
								no = (char)(mai - card_use[USE_SVC]);	/*  制限枚数セット                            */
																		/*                                            */
							switch( CPrmSS[S_SER][(1+3*i)] )			/* ｻｰﾋﾞｽ券役割取得                            */
							{											/*                                            */
								case 1:									/*                                            */
								case 2:									/*                                            */
								case 4:									/*  半額割引処理                              */
									rkn_t 		 	= &req_tkc[req_tkc_cnt];/*  定期中止ﾃﾞｰﾀ要求格納ﾊﾞｯﾌｧ             */
									rkn_t->param 	= RY_CSK;			/*  パラメータセット（精算中止サービス券）    */
									l_dat 		 	= i+1;				/*  種類セット                                */
									rkn_t->data[1] 	= no;				/*	枚数セット                                */
									rkn_t->data[0] 	= l_dat << 16;		/*	種類と掛け売りＮｏ．セット                */
									req_tkc_cnt		++;					/*	定期券中止精算要求納件数ＵＰ              */
									card_use[USE_SVC] += no;			/*                                            */
								break;									/*                                            */
																		/*                                            */
								case 3:									/*                                            */
									rkn_t 		 	= &req_tkc[req_tkc_cnt];/*  定期中止ﾃﾞｰﾀ要求格納ﾊﾞｯﾌｧ         	  */
									rkn_t->param 	= RY_CSK;			/*  パラメータセット（精算中止サービス券）    */
									l_dat 		 	= i+1;				/*  種類セット                                */
									rkn_t->data[1] 	= no;				/*	枚数セット                                */
									rkn_t->data[0] 	= l_dat << 16;		/*	種類と掛け売りＮｏ．セット                */
									req_tkc_cnt		++;					/*	定期券中止精算要求納件数ＵＰ              */
								break;									/*                                            */
							}											/*                                            */
						}												/*                                            */
					}													/*                                            */
				}														/*                                            */
				if(( tki_cyusi.dt[tkcyu_ichi-1].kry != 0 )				/*	料金差引割り引き有り？                    */
						&&(CPrmSS[S_PRP][1] != 0L))						/*  回数券orPｶｰﾄﾞ使用あり					*/
				{														/*                                            */
					rkn_t = &req_tkc[req_tkc_cnt];						/*	定期中止ﾃﾞｰﾀ要求格納ﾊﾞｯﾌｧ                 */
					rkn_t->param = RY_KGK;								/*	パラメータセット（料金差引割り引き）      */
					rkn_t->data[0] = tki_cyusi.dt[tkcyu_ichi-1].kry;	/*	金額セット                                */
					rkn_t->data[1]= 0L;									/*  とりあえず回数券使用可=0とする			*/
					if( CPrmSS[S_PRP][1] == 1L )						/*	設定がPｶｰﾄﾞ使用可 =1					*/
						rkn_t->data[1]= 1L;								/*  Pｶｰﾄﾞ使用可に変更する					*/
					req_tkc_cnt++;										/*	定期券中止精算要求納件数ＵＰ              */
				}														/*                                            */
			}															/*                                            */
																		/*                                            */
		break;															/*                                            */
																		/*                                            */
		case V_PRI:														/*	プリペイドカード                          */
			prk_kind = 1;												/** 基本セット        						  */
			if( tsn_prp.pakno == pnk )			prk_kind = 2;			/** 拡張駐車場№が一致なら  拡張セット        */
			else if( tsn_prp.pakno == pnk2 )	prk_kind = 3;			/** 拡張2駐車場№が一致なら                   */
			else if( tsn_prp.pakno == pnk3 )	prk_kind = 4;			/** 拡張3駐車場№が一致なら                   */
																		/*                                            */
			rkn_p->param = RY_PCO;										/*	パラメータセット                          */
			if( prk_kind == 2 )			rkn_p->param = RY_PCO_K;		/*	拡張駐車Ｎｏ．の場合  パラメータセット    */
			else if( prk_kind == 3 )	rkn_p->param = RY_PCO_K2;		/*  拡張駐車2Ｎｏ．の場合 パラメータセット    */
			else if( prk_kind == 4 )	rkn_p->param = RY_PCO_K3;		/*  拡張駐車3Ｎｏ．の場合 パラメータセット    */
			rkn_p->data[0] = tsn_prp.zan;								/*	残額セット                                */
			rkn_p->data[1] = (unsigned long)tsn_prp.sei;				/*	精算単位セット                            */
																		/*                                            */
			if( prp_rno_cnt < 7 )										/*	ﾌﾟﾘﾍﾟｲﾄﾞカードＮｏ．バッファ数以下        */
			{															/*                                            */
				prp_rno_buf[prp_rno_cnt] = tsn_prp.rno;					/*	ＰＰカードＮｏ．セーブ                    */
				prp_rno_cnt++;											/*	ＰＰカードＮｏ．バッファカウントＵＰ      */
			}															/*                                            */
		break;															/*                                            */
		case V_EMY:														/*	電子マネー		                          */
			prk_kind = 1;												/** 基本セット        						  */
			rkn_p->param = RY_EMY;										/*	パラメータセット                          */
		break;															/*                                            */
// MH810105(S) MH364301 QRコード決済対応
		case V_EQR:														/*	QRコード		                          */
			prk_kind = 1;												/** 基本セット        						  */
			rkn_p->param = RY_EQR;										/*	パラメータセット                          */
			break;														/*                                            */
// MH810105(E) MH364301 QRコード決済対応
		case	V_CRE:									//クレジットカード
			rkn_p->param = RY_CRE;						//計算種別
		break;

		case	V_GNG:													/* 減額精算									  */
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
//// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
//		case 	V_DIS_FEE:												// 遠隔精算中
//		case	V_DIS_TIM:
//// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
			rkn_p->param = RY_GNG;										/* 計算種別									  */
		break;
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
		// 遠隔精算(金額指定)で割引金額あり
		case V_DIS_FEE:
			rkn_p->param = RY_GNG_FEE;
			break;
		// 遠隔精算(入庫時刻指定)で割引金額あり
		case V_DIS_TIM:
			rkn_p->param = RY_GNG_TIM;
			break;
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
		case	V_FRK:													/* 振替精算									  */
			rkn_p->param = RY_FRK;										/* 計算種別									  */
		break;
		case	V_SSS:													/* 修正精算									  */
																		/*											  */
			if( (vl_frs.syubetu != LockInfo[vl_frs.lockno-1].ryo_syu) ){/* 修正元の精算完了時の車種と修正元の設定上の車種が異なる */
				// 車種切換セット										/*											  */
				rysyasu = vl_frs.syubetu;								/* 修正元の車種セット                         */
				
				syashu = rysyasu;										/* 車種セット                                 */
																		/*											  */
				rkn_p->syubt = syashu;									/* 車種セット                                 */
				rkn_p->param = RY_KCH;									/* パラメータセット（車種切換）               */
				sv_tim = Prm_RateDiscTime[syashu-1];					/* 種別時間割引                               */
				rkn_p->data[0] = sv_tim;								/* 種別時間割引セット                         */
				rkn_p->data[1] = 0;										/* 券データとサービス券の種類セット           */
																		/*											  */
				// 振替額セット											/*											  */
				rkn_c = &req_crd[req_crd_cnt];							/* 駐車券読み取り時券データ要求格納バッファ   */
				rkn_c->param = RY_SSS;									/* 計算種別									  */
				req_crd_cnt++;											/* 駐車券読み取り時券データ要求件数ＵＰ       */
			}else{														/*											  */
				rkn_p->param = RY_SSS;									/* 計算種別									  */
			}															/*											  */
		break;															/*											  */
	}																	/*                                            */
																		/*                                            */
	 ans = OK ;															/*	アンサーがＯＫ                            */
																		/*                                            */
	return( ans );														/*                                            */
}																		/**********************************************/
