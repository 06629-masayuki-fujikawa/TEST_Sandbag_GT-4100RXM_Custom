/************************************************************************************/
/*　関数名称　　　: 磁気ﾘｰﾀﾞからのﾃﾞｰﾀ受信制御　　　　		　　　　　　　　 　		*/
/*　関数シンボル　: cr_snd()  　 　　: 　　　　　　　　　　　	　　　　　　: 　	*/
/*　コンパイラ　　: MS-C ver.6.0　	: 　　　　　　　　　　　　　	　　　　: 　	*/
/*　ターゲットCPU : V25   　　　　　: 　　　　　　　　　　　　　　　	　　: 　	*/
/*　作成者（日付）: 戸田			  　　　　　　　　　　　　　　　	　　: 　	*/
/************************************************************************************/
/* Update:2004.5.13  T.nakayama 													*/
/* 	4500EX :	V25->H8 Arcnet 移植	 												*/
/* 		   :	arc_txrx構造体参照方式変更 '->'を '.'に変更							*/
/* 		   :	TF4800N構造とのI/Fをとる											*/
/* 		   :	ｷｭｰｾｯﾄ処理の追加													*/
/* 		   :	EOTｽﾃｰﾀｽ処理の追加 mdltsk.c md_seot()								*/
/************************************************************************************/
#include	<string.h>										/*						*/
#include	"system.h"										/*						*/
#include	"Message.h"										/* Message				*/
#include	"mdl_def.h"										/* ﾓｼﾞｭｰﾙ間通信機器		*/
#include	"mem_def.h"										/*						*/
#include	"pri_def.h"										/*						*/
#include	"rkn_def.h"										/*						*/
#include	"rkn_cal.h"										/*						*/
#include	"ope_def.h"										/*						*/
#include	"tbl_rkn.h"										/*						*/
#include	"prm_tbl.h"
#include	"fbcom.h"
															/*						*/
															/*						*/
void	cr_rcv( void );										/*						*/
char	cr_queset( char );									/*						*/
void	avm_rcv( void );									/*						*/
char	avm_queset( char);									/*						*/
															/*						*/
static	void 	md_seot(unsigned char *, short );			/*						*/
															/*						*/
char	MTS_rseq[MTS_MAX] = {(char)-1, (char)-1};			/*						*/
uchar	MRD_VERSION[16];									// 磁気リーダーバージョン
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//extern	char	pcard_shtter_ctl;
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
															/*						*/
void	cr_rcv( void )										/*						*/
{															/*						*/
	char	seq;											/*						*/

	seq = FBcom_RcvData[1];									/* ﾓｼﾞｭｰﾙ間のｼｰｹﾝｼｬﾙ№	*/
	switch( seq )											/*						*/
	{														/*						*/
	case 0x04:												/* EOT					*/
		cr_queset(seq);										/* ｷｭｰｾｯﾄ				*/
		break;												/*						*/
															/*						*/
	case '1':												/*						*/
	case '2':												/*						*/
	case '3':												/*						*/
	case '4':												/*						*/
	case '5':												/*						*/
	case '6':												/*						*/
	case '7':												/*						*/
	case '8':												/*						*/
	case '9':												/*						*/
		if( seq == MTS_rseq[MTS_RED] )						/* 同一ｼｰｹﾝｼｬﾙ№		*/
		{													/*						*/
			err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 0, 0, 0 );	/* 00:ﾘｰﾀﾞ通信異常解除	*/
			return;											/*						*/
		}													/*						*/
															/*						*/
	case '0':												/*						*/
		err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 0, 0, 0 );	/* 00:ﾘｰﾀﾞ通信異常解除	*/
		MTS_rseq[MTS_RED] = seq;							/*						*/
															/*						*/
		cr_queset(seq);										/* ｷｭｰｾｯﾄ				*/
// MH810103 GG119202(S) ICクレジット対応（PCI-DSS）
		memset(FBcom_RcvData, 0, sizeof(FBcom_RcvData));
// MH810103 GG119202(E) ICクレジット対応（PCI-DSS）
	}														/*						*/
	return;													/*						*/
}															/*						*/
															/*						*/
/*==================================================================================*/

/*[]------------------------------------------------------------------------------[]*/
/*| Reader ARCNET受信ﾃﾞｰﾀをｷｭｰへｾｯﾄする                                            |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Argument : arc_txrx.rxARCdata[1]:ｼｰｹﾝｼｬﾙ№	                        		   |*/
/*|          : arc_txrx.rxARCdata[3]:ﾃﾞｰﾀﾌﾞﾛｯｸ№		                   		   |*/
/*|          : arc_txrx.rxARCdata[4]:ﾃﾞｰﾀｻｲｽﾞ			                   		   |*/
/*|          : arc_txrx.rxARCdata[5]:ﾃﾞｰﾀ種別			                   		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Return : queset											              		   |*/
/*|        : ARC_CR_R_EVT: ﾘｰﾄﾞｺﾏﾝﾄﾞ受信 R				                   		   |*/
/*|        : ARC_CR_E_EVT: 動作結果ｺﾏﾝﾄﾞ受信 E			                   		   |*/
/*|        : ARC_CR_VER_EVT: ﾊﾞｰｼﾞｮﾝ受信 0x09			                   		   |*/
/*|        : 											                   		   |*/
/*|        :  0: 						 				                  		   |*/
/*|        : -1:Ireagual Data			 				                  		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Author      :  T.Nakayama                                              		   |*/
/*| Date        :  2004-05-19                                              		   |*/
/*| Update      :                                                          		   |*/
/*[]---------------------------------------------- Copyright(C) 2004 AMANO Corp.--[]*/
char	cr_queset( char seq )								/*						*/
{															/*						*/
	unsigned short evt=0;									/*						*/
															/*						*/
	if( seq == 0x04 ){										/* Eot					*/
															/*						*/
		md_seot( &FBcom_RcvData[2], 0 );
															/* 						*/
		/*--------------------------------------------------------------------------*/
		/* 挿入口券ありビットがON→OFF時のイベント									*/
		/*--------------------------------------------------------------------------*/
		w_stat1[0] = w_stat1[1];							/*						*/
		w_stat1[1] = w_stat1[2];							/*						*/
		w_stat1[2] =										/* 挿入口券ｱﾘ New Status*/
			(unsigned char)( RED_stat[2] & TICKET_MOUTH );	/*						*/
															/*						*/
		/*--------------------------------------------------------------------------*/
		/* Rコマンド受信待ちイベント、Rコマンド受信イベント							*/
		/*--------------------------------------------------------------------------*/
		if( w_stat2 == 0x00 ){								/* Rｺﾏﾝﾄﾞ受信待ち?(Y)	*/
		   if( (RED_stat[3] & R_CMD_RCV) == R_CMD_RCV  ){	/* Rｺﾏﾝﾄﾞ受信済み?(Y)	*/
															/* 						*/
			queset( OPETCBNO, ARC_CR_EOT_RCMD, 0, NULL ); 	/* 24 					*/
			}												/*						*/
		}else{												/* Rｺﾏﾝﾄﾞ受信済み?(Y)	*/
		   if( (RED_stat[3] & R_CMD_RCV) == 0 ){			/* Rｺﾏﾝﾄﾞ受信待ち		*/
															/* 						*/
			queset( OPETCBNO, ARC_CR_EOT_RCMD_WT, 0, NULL );/* 25 					*/
			}												/*						*/
		}													/*						*/
		w_stat2 =											/* Rｺﾏﾝﾄﾞ受信 New Status*/
			(unsigned char)(RED_stat[3] & R_CMD_RCV);		/*						*/
															/*						*/
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//// MH321800(S) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
//		if(	( ( RED_stat[2] & TICKET_MOUTH) == TICKET_MOUTH ) &&//挿入口券あり
//			( ( RED_stat[3] & 0x20) != 0x20 ) ){				//カード挿入待ちではない
//			if( pcard_shtter_ctl == 1 ){
//				queset( OPETCBNO, ARC_CR_EOT_MOVE_TKT, 0, NULL );	/* 30 			*/
//			}
//		}
//// MH321800(E) T.Nagai Pカードベリファイ時にシャッター閉する不具合対応(FCR.P170096)(MH341110流用)
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
															/*						*/
	}else{													/*						*/
		/*--------------------------------------------------------------------------*/
		/* TF4800Nの構造体に、ｱｰｸﾈｯﾄで受信したﾃﾞｰﾀをｺﾋﾟｰする						*/
		/*--------------------------------------------------------------------------*/
		if( FBcom_RcvData[5] == FB_RCV_READ_ID){			/* D Read Data			*/
			evt = ARC_CR_R_EVT;								/* 20					*/
															/*						*/
			RDT_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RDT_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			memcpy( &RDT_REC.rdat[0],						/*						*/
					&FBcom_RcvData[5],						/*						*/
					(size_t)FBcom_RcvData[4]);				/*						*/
															/*						*/
															/*						*/
		}else if( FBcom_RcvData[5] == FB_RCV_END_ID){		/* E					*/
			evt = ARC_CR_E_EVT;								/* 21					*/
															/*						*/
			RED_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RED_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			RED_REC.ecod = FBcom_RcvData[5];				/* 						*/
			RED_REC.ercd = FBcom_RcvData[6];				/* Error Code			*/
															/*						*/
			RED_REC.posi[0] = FBcom_RcvData[7];				/* 						*/
			RED_REC.posi[1] = FBcom_RcvData[8];				/* 						*/
															/*						*/
															/*						*/
		}else if( FBcom_RcvData[5] == FB_RCV_VERS_ID){		/* Version				*/
			evt = ARC_CR_VER_EVT;							/* 22					*/
															/*						*/
			RVD_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RVD_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			RVD_REC.kind = FBcom_RcvData[5];				/*						*/
															/*						*/
			RVD_REC.vers[0] = FBcom_RcvData[6];				/* Ver					*/
			RVD_REC.vers[1] = FBcom_RcvData[7];				/* Ver					*/
															/*						*/
		}else if( (FBcom_RcvData[5] == FB_RCV_MNT_ID) &&	/* ﾒﾝﾃﾅﾝｽｺﾏﾝﾄﾞ応答		*/
				  (FBcom_RcvData[6] == 0x01) ){				/* ﾒﾝﾃﾅﾝｽｺｰﾄﾞ=部番		*/

			evt = ARC_CR_VER_EVT;							/* 22					*/
															/*						*/
			RVD_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RVD_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			RVD_REC.kind = FBcom_RcvData[5];				/*						*/
															/*						*/
			memcpy( RVD_REC.vers , &FBcom_RcvData[10] , 8);	/*						*/
			memcpy(MRD_VERSION, &FBcom_RcvData[10] , 8);	// 磁気リーダーバージョン
		}else if( (FBcom_RcvData[5] == FB_RCV_MNT_ID) &&	/* ﾒﾝﾃﾅﾝｽｺﾏﾝﾄﾞ応答		*/
				  (FBcom_RcvData[6] == 0x09) ){				/* ﾒﾝﾃﾅﾝｽｺｰﾄﾞ=部番		*/
			evt = ARC_CR_E_EVT;								/* 22					*/

			RDS_REC.result = FBcom_RcvData[9];				/* 取得結果				*/
			RDS_REC.state[0] = FBcom_RcvData[10];			/* ディップスイッチ1状態*/
			RDS_REC.state[1] = FBcom_RcvData[11];			/* ディップスイッチ2状態*/
			RDS_REC.state[2] = FBcom_RcvData[12];			/* ディップスイッチ3状態*/
			RDS_REC.state[3] = FBcom_RcvData[13];			/* ディップスイッチ4状態*/

		}else{												/*						*/
			return (char)-1;								/*						*/
		}													/*						*/
															/*						*/
		if( evt != 0 ) {									/*						*/
			queset( OPETCBNO, evt, 0, NULL );				/* ｷｭｰｾｯﾄ				*/
		}
															/*						*/
	}														/*						*/
	return(0);												/*						*/
}															/*						*/
															/*						*/
/*[]-------------------------------------------------------------------[]*/
/*|	Set Module Status ( EOT ) TF4800N									|*/
/*[]-------------------------------------------------------------------[]*/
/*| MODULE NAME  : md_seot( ptr, no );									|*/
/*| PARAMETER	 : char		*ptr	: Status Data Address				|*/
/*|		   int		no;	: Module No. ( 0 - 2 )							|*/
/*| RETURN VALUE : none													|*/
/*|              : RED_stat更新											|*/
/*[]-------------------------------------------------------------------[]*/
/*| Update	   : 2004.5.19 T.Nakayama									|*/
/*| 		   :	EOTｽﾃｰﾀｽ処理の追加 mdltsk.c md_seot()				|*/
/*[]-------------------------------------------------------------------[]*/
static	void md_seot	( unsigned char *ptr, short no )
{
	unsigned char	*cp;
	short	size;
	unsigned char set;
	unsigned char alm;

	switch( no ) {
	    case 0:												/* 磁気ﾘｰﾀﾞｰ 			*/
		if( RD_mod < 6 ) {									/* ｲﾆｼｬﾙ未完了は除く	*/
			return;											/*						*/
		}													/*						*/
		set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
		if(set == 1 || set == 2) {
			alm = ptr[0];
		}
		else {
			// プリンタなしの場合はプリンタのアラームを除外する
			alm = (unsigned char)(ptr[0] & 0x7f);
		}
		cp = RED_stat;										/* EOT status			*/
		if( cp[0] == 0 ) {									/*						*/
			if( alm != 0 ) {								/* Alarm Happen ?		*/
				alm_chk( ALMMDL_SUB, ALARM_READERSENSOR, 1 );/* ｱﾗｰﾑﾛｸﾞ登録解除		*/
			}												/*						*/
		} else {											/*						*/
			if( alm == 0 ) {								/*						*/
				alm_chk( ALMMDL_SUB, ALARM_READERSENSOR, 0 );/* ｱﾗｰﾑﾛｸﾞ登録解除		*/
			}												/*						*/
		}													/*						*/
		err_mod_chk( (uchar*)ptr, (uchar*)cp, 0 );			/* ﾓｼﾞｭｰﾙ機器ｴﾗｰﾁｪｯｸ	*/
															/* Reader				*/
		size = 4;											/* RED_stat size		*/
		break;												/*						*/
															/*						*/
	    default:
		return;
	}
	memcpy( cp, ptr, (size_t)size );						/* RED_stat更新			*/

	return;
}

/*[]------------------------------------------------------------------------------[]*/
/*|	音声放送用受信処理															   |*/
/*|	音声放送用IBJからARCNETデータを受信し、データ処理する						   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : avm_rcv()													   |*/
/*| PARAMETER	 : void															   |*/
/*| RETURN VALUE : none															   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| date	   : 2004.10.15 T.Nakayama											   |*/
/*| 		   : 																   |*/
/*[]------------------------------------------------------------------------------[]*/
void	avm_rcv( void )										/*						*/
{															/*						*/
	return;													/*						*/
}															/*						*/
															/*						*/
/*==================================================================================*/

/*[]------------------------------------------------------------------------------[]*/
/*| 音声放送 ARCNET受信ﾃﾞｰﾀをｷｭｰへｾｯﾄする                                          |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Argument : arc_txrx.rxARCdata[1]:ｼｰｹﾝｼｬﾙ№	                        		   |*/
/*|          : arc_txrx.rxARCdata[3]:ﾃﾞｰﾀﾌﾞﾛｯｸ№		                   		   |*/
/*|          : arc_txrx.rxARCdata[4]:ﾃﾞｰﾀｻｲｽﾞ			                   		   |*/
/*|          : arc_txrx.rxARCdata[5]:ﾃﾞｰﾀ種別			                   		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Return : queset											              		   |*/
/*|        : ARC_AVM_EXEC_EVT: 音声放送実行/解除		                   		   |*/
/*|        : ARC_AVM_VER_EVT:  ﾊﾞｰｼﾞｮﾝ受信				                   		   |*/
/*|        : 											                   		   |*/
/*|        :  0: 						 				                  		   |*/
/*|        : -1:Ireagual Data			 				                  		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Author      :  T.Nakayama                                              		   |*/
/*| Date        :  2004-10-15                                              		   |*/
/*| Update      :                                                          		   |*/
/*[]---------------------------------------------- Copyright(C) 2004 AMANO Corp.--[]*/
char	avm_queset( char seq )								/*						*/
{															/*						*/
	return(0);												/*						*/
}															/*						*/
