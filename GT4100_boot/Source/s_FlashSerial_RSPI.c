//[]----------------------------------------------------------------------[]
///	@file		s_FlashSerial_RSPI.c
///	@brief		ブートプログラム用 シリアルフラッシュROM SPIドライバ
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include "iodefine.h"
#include "s_FlashSerial.h"
/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void Rspi_from_Init(void);
static void spi_wait(void);
unsigned char FlashReadData_direct(const unsigned long , unsigned char *, const unsigned long );

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

#pragma section F_UPDATE_FUNC	// プログラム書き込み時プログラムを内蔵RAMにコピーする領域

//[]----------------------------------------------------------------------[]
///	@brief			ソフトウェアウェイト SPI用
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void spi_wait(void)
{
	unsigned int count = 5;
	while(count > 0) {
		count--;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			リードコマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: リード開始開始アドレス
///	@param[out]		*data	: リードデータを書き込むポインタ
///	@param[in]		size	: リードデータサイズ(1～4096(1セクタ))
///	@return			0		: 異常終了
///					1		: 正常終了
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadData_direct(const unsigned long address, unsigned char *data, const unsigned long size)
{
	unsigned char	command[4];
	unsigned long i ;
	volatile unsigned char	dmmy;

	command[0] = 0x3;						// Read
	command[1] = (unsigned char)(address >> 16);
	command[2] = (unsigned char)(address >>  8);
	command[3] = (unsigned char)(address >>  0);

	// コマンド送信フェーズ
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// 転送の途中の時 SSL信号はアクティブを保持
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
	for (i = 0; i < 4; i++)
	{
		RSPI0.SPDR = command[i];
		while (RSPI0.SPSR.BIT.IDLNF);
		spi_wait();							// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
		dmmy = (unsigned char) RSPI0.SPDR;	// ダミーリード
	}
	// データ受信フェーズ
	for (i = 0; i < size; i++) {
		if(i == (size-1)) {
			RSPI0.SPCMD0.WORD = 0x2703;		// 転送の最後の時 SSL信号は転送完了で解除
		}
		RSPI0.SPDR = 0xFF;
		/* Wait for transfer to complete */
		while (RSPI0.SPSR.BIT.IDLNF) ;		// ensure transmit register is empty
		spi_wait();							// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
		data[i] = (unsigned char) (RSPI0.SPDR & 0xFF) ;
	}
	return (unsigned char)1;				// 正常終了
}

