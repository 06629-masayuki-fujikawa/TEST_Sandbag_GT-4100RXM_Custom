//[]----------------------------------------------------------------------[]
///	@file		FlashSerial_RSPI.c
///	@brief		シリアルフラッシュROM SPIドライバ
/// @date		2012/02/09
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include "iodefine.h"
#include "system.h"
#include "FlashSerial.h"
/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void Rspi_from_Init(void);
void Rspi_from_SPRI( void );
unsigned char Rspi_from_SndReq( unsigned short Length );
unsigned char Rspi_from_IsSndRcvCmp( void );

/*--- Define --------------------*/

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			シリアルフラッシュROM SPIドライバ初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Rspi_from_Init(void)
{
	// 受信管理情報を初期化
	Rspi_RcvCtrl.RcvReqCnt  = 0;
	Rspi_RcvCtrl.RcvCmpCnt  = 0;
	Rspi_RcvCtrl.RcvCmpFlg  = 1;

	// 送信管理情報を初期化
	Rspi_SndCtrl.SndReqCnt = 0;
	Rspi_SndCtrl.SndCmpCnt = 0;
	Rspi_SndCtrl.ReadIndex = 0;
	Rspi_SndCtrl.SndCmpFlg = 1;

	memset(Rspi_RcvBuf,0xFF,sizeof(Rspi_RcvBuf));
	memset(Rspi_SndBuf,0xFF,sizeof(Rspi_SndBuf));
	From_Access_SemFree(0);		// FROMアクセス権は開放しておく

	// RSPI0の初期設定
	MSTP(RSPI0) = 0;
	RSPI0.SSLP.BYTE = 0x00;		// SSL0P～SSL3PはLアクティブ(デフォルト)
	RSPI0.SPPCR.BYTE = 0x00;	// (デフォルト)
	RSPI0.SPSCR.BYTE = 0x00;	// (デフォルト)
	RSPI0.SPBR.BYTE = 0x01;		// 12.5Mbps
	RSPI0.SPDCR.BYTE = 0x20;	// 32-bit data, 1 frame
	RSPI0.SPCKD.BYTE = 0x00;	// (デフォルト)
	RSPI0.SSLND.BYTE = 0x00;	// (デフォルト)
	RSPI0.SPND.BYTE = 0x00;		// (デフォルト)
	// SPCMDのSSL信号保持ビットは転送するデータ長によって変更する必要があります。(初期値は転送終了でネゲートとする)
	RSPI0.SPCMD0.WORD = 0x2703;	// b15:SCKDEN=0(RSPCK遅延は1RSPCLK)
								// b14:SLNDEN=0(SSLネゲート遅延は1RSPCK)
								// b13:SPNDEN=1(次アクセス遅延はRSPI次アクセス遅延レジスタ（SPND）の設定値)
								// b12:LSBF=0(MSBファースト)
								// b11-8:SPB=111(8bit)
								// b7:SSL信号レベル保持ビットSSLKP=0(転送終了時に全SSL信号をネゲート)
								// b6-4:SSLA=000(SSL0)
								// b3-2:BRDV=00(ベースのビットレートを選択)
								// b1:CPOL=1(アイドル時のRSPCKが“1”)
								// b0:CHPA=1(奇数エッジでデータ変化、偶数エッジでデータサンプル)
								// モード3

	RSPI0.SPCR.BYTE = 0x48;		// Enable RSPI in master mode

}

//[]----------------------------------------------------------------------[]
///	@brief			シリアルフラッシュROM SPI受信割り込み処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
/// クロック同期のため送信と受信はセットとなっている。
/// 送信／受信の割り込みを別々に制御するとオーバーランが発生する可能性が高い
/// 受信割り込みで受信データを取り込んだあとで、送信処理を行なえば転送速度は若干遅く
/// (約90％)なるがオーバーランエラーは発生しない。
/// そのために受信割り込み処理のみで送受信の両方の制御を行う
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Rspi_from_SPRI( void )
{

// 受信処理
	Rspi_RcvBuf[Rspi_RcvCtrl.RcvCmpCnt] = (unsigned char) (RSPI0.SPDR & 0xFF) ;	// 受信バッファへ転送後、受信データ数カウントアップ
	Rspi_RcvCtrl.RcvCmpCnt++;
	if (Rspi_RcvCtrl.RcvReqCnt <= Rspi_RcvCtrl.RcvCmpCnt) {
		RSPI0.SPCR.BIT.SPRIE = 0;												// RSPI受信割り込み禁止
		Rspi_RcvCtrl.RcvCmpFlg = 1;												// 受信完了した(完了フラグ←ON)
	}
// 送信処理
	Rspi_SndCtrl.SndCmpCnt++;													// 送信完了データ数カウント（アップ）
	if (Rspi_SndCtrl.SndReqCnt > Rspi_SndCtrl.SndCmpCnt) {						// 送信データ有
		if(Rspi_SndCtrl.SndReqCnt == (Rspi_SndCtrl.SndCmpCnt + 1)){				// 転送の最後の時
			RSPI0.SPCMD0.WORD = 0x2703;// SSL信号は転送完了で解除
		}
		RSPI0.SPDR = Rspi_SndBuf[ Rspi_SndCtrl.ReadIndex++];					// 送信バッファから転送 
	}
	else {
		Rspi_SndCtrl.SndCmpFlg = 1;												// 送信完了した(完了フラグ←ON)
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			シリアルフラッシュROM SPI送信要求処理
//[]----------------------------------------------------------------------[]
///	@param[in]		Length	: 転送データ長1～RSPI_BUF_SIZE
///	@return			0		: 異常終了
///					1		: 正常終了
///	@author			A.iiizumi
///	@note			送信バッファRspi_SndBuf[]にデータをセットしてからコールすること
///					受信したデータはRspi_RcvBuf[]に格納されます
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char Rspi_from_SndReq( unsigned short Length )
{
	if ((Length == 0)||(Length > RSPI_BUF_SIZE)) {// セーフティ機能 
		Rspi_SndCtrl.SndCmpFlg = 1;				// 送信完了した(完了フラグ←ON)
		Rspi_RcvCtrl.RcvCmpFlg = 1;				// 受信完了した(完了フラグ←ON)
		return (unsigned char)0;				// 異常終了（送信開始できなかった）
	}

	if( 0 == RSPI0.SPSR.BIT.IDLNF ){			// SPI0がアイドル状態
		Rspi_SndCtrl.SndCmpFlg = 0;				// 送信開始(完了フラグ←OFF)
		Rspi_SndCtrl.SndReqCnt = Length;		// 送信要求データ数をセット
		Rspi_SndCtrl.SndCmpCnt = 0;				// 送信完了データ数をリセット
		Rspi_SndCtrl.ReadIndex = 1;				// 最初の１文字転送(インデックスのセット)

		Rspi_RcvCtrl.RcvCmpFlg = 0;				// 送信開始(完了フラグ←OFF)
		Rspi_RcvCtrl.RcvReqCnt = Length;		// 送信要求データ数をセット
		Rspi_RcvCtrl.RcvCmpCnt = 0;				// 送信完了データ数をリセット
		if(Length == 1){// 転送が1バイトの時
			RSPI0.SPCMD0.WORD = 0x2703;			// 転送の最後より SSL信号は転送完了で解除
		}else {
			RSPI0.SPCMD0.WORD = 0x2783;			// 転送の途中より SSL信号はアクティブを保持
		}
		// NOTE:必ず割り込み許可をしてからSPDRにデータをセットすること(割り込みを取りこぼすため)
		RSPI0.SPCR.BIT.SPRIE = 1;				// RSPI受信割り込み許可
		RSPI0.SPDR = Rspi_SndBuf[0];			// 最初の１文字を送信バッファから転送
		return (unsigned char)1;				// 正常終了（送信開始できた）
	}
	else {										// SPI送受信動作中
		Rspi_SndCtrl.SndCmpFlg = 1;				// 送信完了した(完了フラグ←ON)
		Rspi_RcvCtrl.RcvCmpFlg = 1;				// 受信完了した(完了フラグ←ON)
		RSPI0.SPCR.BIT.SPRIE = 0;				// RSPI受信割り込み禁止
		return (unsigned char)0;				// 異常終了（送信開始できなかった）
	}
	return (unsigned char)1;
}
//[]----------------------------------------------------------------------[]
///	@brief			シリアルフラッシュROM SPI送受信完了判定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0		: 送受信未完了
///					1		: 送受信完了
///	@author			A.iiizumi
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char	Rspi_from_IsSndRcvCmp( void )
{
	if((1 == Rspi_RcvCtrl.RcvCmpFlg)&&
	   (1 == Rspi_SndCtrl.SndCmpFlg)){
		return (unsigned char)1;
	}
	return (unsigned char)0;
}
