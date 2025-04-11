//[]----------------------------------------------------------------------[]
///	@file		FlashSerial_RSPI.c
///	@brief		シリアルフラッシュROM アクセスAPI
/// @date		2012/02/09
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*--- Include -------------------*/
#include "iodefine.h"
#include "system.h"
#include "FlashSerial.h"
#include "mdl_def.h"
/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void SetRsten(void);
void SetRst(void);
unsigned char FlashReadStatus(void);
void FlashWriteStatus( unsigned char );
unsigned char  FlashErase ( const unsigned long, const unsigned char );
unsigned char  FlashReadData ( const unsigned long, unsigned char *, const unsigned long );
void FlashWriteData ( unsigned long, unsigned char const *);
void FlashJEDECReadId ( unsigned char * );

unsigned char FlashReadStatus_direct(void);
void FlashWriteStatus_direct(unsigned char );
unsigned char FlashErase_direct(const unsigned long , const unsigned char );
void FlashWriteData_direct(unsigned long , unsigned char const *);
unsigned char FlashReadData_direct(const unsigned long , unsigned char *, const unsigned long );
void FlashJEDECReadId_direct(unsigned char *);
unsigned char From_Access_SemGet(unsigned char FlashSodiac);
void From_Access_SemFree(unsigned char FlashSodiac);
static void spi_wait(void);
static void WriteEnable(void);
static void WriteEnable_direct(void);
static unsigned char	f_from_access_sem;			// SPI FROMアクセスフラグ 0:アクセス権開放状態 1:アクセス件取得状態

extern		unsigned char			Sodiac_FROM_Get;
extern		char use_avm;

/*--- Define --------------------*/

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/
//[]----------------------------------------------------------------------[]
///	@brief			SPI FROMアクセス権取得（セマフォ取得）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0: アクセス権取得失敗
///					1: アクセス権取得成功
///	@author			A.iiizumi
///	@attention		SPI FROMにアクセスする前にフラグを取得する。
///					取得できない場合はタスク切り換えを行いながら取得可能となるまで待つこと。
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char From_Access_SemGet(unsigned char FlashSodiac)
{	//	0/1/2 = none/Flash/Sodiac
	if(FlashSodiac == 1){
		/* ch0, ch1どちらか再生中の場合	*/
		if(use_avm == AVM_CONNECT){												// AVM接続有り
			if( (0 == AVM_Sodiac_Ctrl[0].play_message_cmp) || ( 0 == AVM_Sodiac_Ctrl[1].play_message_cmp) )
			{
				return(unsigned char)0;// 取得失敗
			}
		}
	}
	if(FlashSodiac == 2){
		if(Rspi_from_IsSndRcvCmp() == 0){										// RSPI送受信中
			return(unsigned char)0;			// 取得失敗
		}
	}
	if(f_from_access_sem == 0){ // fromにアクセスしていない
		f_from_access_sem = 1; // アクセスフラグセット
		Sodiac_FROM_Get = FlashSodiac;
		return(unsigned char)1;// 取得成功
	}
	return(unsigned char)0;// 取得失敗
}

//[]----------------------------------------------------------------------[]
///	@brief			SPI FROMアクセス権開放（セマフォ開放）
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		SPI FROMのアクセス権を開放する
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void From_Access_SemFree(unsigned char FlashSodiac)
{	//	0/1/2 = none/Flash/Sodiac
	if (Sodiac_FROM_Get == FlashSodiac) {
		f_from_access_sem = 0; // アクセスフラグクリア
		Sodiac_FROM_Get = 0;
	}
}
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
///	@brief			RSTENコマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			NMI発生時にFROMをリセットするためのコマンド(RSTEN,RSTセットで実行する)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void SetRsten(void)
{
	unsigned char command = 0x66;			// RSTEN
	// コマンド送信フェーズ
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2703;				// 転送の最後の時 SSL信号は転送完了で解除
	RSPI0.SPDR = command;
	while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
	command = (unsigned char) RSPI0.SPDR;	// ダミーリード
}
//[]----------------------------------------------------------------------[]
///	@brief			RSTコマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			NMI発生時にFROMをリセットするためのコマンド(RSTEN,RSTセットで実行する)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void SetRst(void)
{
	unsigned char command = 0x99;			// RST
    // コマンド送信フェーズ
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2703;				// 転送の最後の時 SSL信号は転送完了で解除
	RSPI0.SPDR = command;
	while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
	command = (unsigned char) RSPI0.SPDR;	// ダミーリード
}

//[]----------------------------------------------------------------------[]
///	@brief			WREN(ライトイネーブル)コマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		通常動作で使用(割り込みによる送受信:タスク切り換え動作が始まってから使用するもの)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void WriteEnable(void)
{
	Rspi_SndBuf[0] = 0x06;					// WREN
	Rspi_from_SndReq(1);					// 送信1byteで要求
}
//[]----------------------------------------------------------------------[]
///	@brief			RDSR(ステータスリード)コマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			stat	: シリアルFROMのステータス
///	@author			A.iiizumi
///	@attention		通常動作で使用(割り込みによる送受信:タスク切り換え動作が始まってから使用するもの)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadStatus(void)
{
	unsigned char stat;
	stat = 0xFF;

	Rspi_SndBuf[0] = 0x05;					// RDSR
	Rspi_SndBuf[1] = 0xFF;					// 1Byte受信するのでクロックを送信するためダミーデータセット
	Rspi_from_SndReq(2);					// 送信1byte,受信1byteで要求

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	stat = 	Rspi_RcvBuf[1];					// 受信データの2Byte目がステータス
	return(stat);
}

//[]----------------------------------------------------------------------[]
///	@brief			WRSR(ステータスライト)コマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		val		: シリアルFROMのステータス
///	@return			void
///	@author			A.iiizumi
///	@attention		通常動作で使用(割り込みによる送受信:タスク切り換え動作が始まってから使用するもの)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteStatus(unsigned char val)
{
	WriteEnable();							// WREN(ライトイネーブル)
	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	Rspi_SndBuf[0] = 0x01;					// WRSR
	Rspi_SndBuf[1] = val;					// 設定データ
	Rspi_from_SndReq(2);					// 送信2byteで要求

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	/* check status to make sure previous operation completed */
	while( FlashReadStatus()&SR_BSY );
}
//[]----------------------------------------------------------------------[]
///	@brief			イレースコマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: イレース開始アドレス(オールイレースの場合は任意)
///	@param[in]		size	: イレース内容 0:Chip,1:4KB(sector),2:64KB(block)
///	@return			0		: 異常終了
///					1		: 正常終了
///	@author			A.iiizumi
///	@attention		通常動作で使用(割り込みによる送受信:タスク切り換え動作が始まってから使用するもの)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashErase(const unsigned long address, const unsigned char size)
{
	const unsigned char	erase_commands[] = { 0xc7, 0x20, 0xd8 };	// Chip, 4KB, 64KB

	if(size > 2){
		return (unsigned char)0;			// 要求間違いの場合何もしない
	}

	WriteEnable();							// WREN(ライトイネーブル)
	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	Rspi_SndBuf[0] = erase_commands[size];	// コマンドセット
	if( size>0 ) {	// followed address
		Rspi_SndBuf[1] = (unsigned char)(address >> 16);
		Rspi_SndBuf[2] = (unsigned char)(address >>  8);
		Rspi_SndBuf[3] = (unsigned char)(address >>  0);
		Rspi_from_SndReq(4);				// 送信4byteで要求
	}
	else {
		Rspi_from_SndReq(1);				// 送信1byteで要求
	}

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	/* check status to make sure previous operation completed */
	while( FlashReadStatus()&SR_BSY );

	return (unsigned char)1;				// 正常終了
}
//[]----------------------------------------------------------------------[]
///	@brief			ライトコマンド
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: ライト開始開始アドレス
///	@param[in]		*data	: 書き込むデータのポインタ(書き込みサイズは256Byte単位固定) 
///	@return			void
///	@author			A.iiizumi
///	@attention		通常動作で使用(割り込みによる送受信:タスク切り換え動作が始まってから使用するもの)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteData(unsigned long address, unsigned char const *data)
{
	// 書き込みサイズは256Byte単位の仕様
	WriteEnable();							// WREN(ライトイネーブル)
	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	memcpy(&Rspi_SndBuf[4],data,256);		// 送信データを256Byte分コピー
	Rspi_SndBuf[0] = 0x02;					// Program
	Rspi_SndBuf[1] = (unsigned char)(address >> 16);
	Rspi_SndBuf[2] = (unsigned char)(address >>  8);
	Rspi_SndBuf[3] = (unsigned char)(address >>  0);
	Rspi_from_SndReq(260);					// 送信260byteで要求(コマンド4,データ256)

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	/* check status to make sure previous operation completed */
	while( FlashReadStatus()&SR_BSY );
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
///	@attention		通常動作で使用(割り込みによる送受信:タスク切り換え動作が始まってから使用するもの)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadData(const unsigned long address, unsigned char *data, const unsigned long size)
{
	unsigned long  data_size;
	unsigned int i ;

	if((size == 0) || (FROM_SECTOR_SIZE < size)) {
		return (unsigned char)0;			// 範囲外の場合は何もしない
	}

	memset(Rspi_SndBuf,0xFF,sizeof(Rspi_SndBuf));
	Rspi_SndBuf[0] = 0x03;					// Read
	Rspi_SndBuf[1] = (unsigned char)(address >> 16);
	Rspi_SndBuf[2] = (unsigned char)(address >>  8);
	Rspi_SndBuf[3] = (unsigned char)(address >>  0);
	data_size = size + 4;
	Rspi_from_SndReq(data_size);			// 送信260byteで要求(コマンド4,データ256)

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	for (i = 0; i < size; i++) {
		data[i] = Rspi_RcvBuf[i+4];			// 受信データの4Byte目からがデータ
	}
	return (unsigned char)1;				// 正常終了
}
//[]----------------------------------------------------------------------[]
///	@brief			device identification リードコマンド
//[]----------------------------------------------------------------------[]
///	@param[out]		*val	: リードデータを書き込むポインタ(3byte)
///	@return			void
///	@author			A.iiizumi
///	@attention		通常動作で使用(割り込みによる送受信:タスク切り換え動作が始まってから使用するもの)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashJEDECReadId(unsigned char *val)
{
	int i ;

	Rspi_SndBuf[0] = 0x9F;					// JEDEC-ID
	Rspi_SndBuf[1] = 0xFF;					// 3Byte受信するのでクロックを送信するためダミーデータセット
	Rspi_SndBuf[2] = 0xFF;
	Rspi_SndBuf[3] = 0xFF;
	Rspi_from_SndReq(4);					// 送信1byte,受信3byteで要求

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	for (i = 0; i < 3; i++) {
		val[i] = Rspi_RcvBuf[i+1];			// 受信データの2Byte目からがデータ
	}

}


//[]----------------------------------------------------------------------[]
///	@brief			WREN(ライトイネーブル)コマンド(割り込み未使用)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		起動時のFROMリカバリ処理で使用すること(タスク切り替えが始まってからは使用禁止)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void WriteEnable_direct(void)
{
	unsigned char command = 0x06;			// WREN

    // コマンド送信フェーズ
    while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2703;				// 転送の最後の時 SSL信号は転送完了で解除
    RSPI0.SPDR = command;
    while (RSPI0.SPSR.BIT.IDLNF) ;
   	command = (unsigned char) RSPI0.SPDR;	// ダミーリード
}

//[]----------------------------------------------------------------------[]
///	@brief			RDSR(ステータスリード)コマンド(割り込み未使用)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			stat	: シリアルFROMのステータス
///	@author			A.iiizumi
///	@attention		起動時のFROMリカバリ処理で使用すること(タスク切り替えが始まってからは使用禁止)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadStatus_direct(void)
{
	unsigned char command = 0x05;			// RDSR
	volatile unsigned char dmmy;

    // コマンド送信フェーズ
    while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// 転送の途中の時 SSL信号はアクティブを保持
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
    RSPI0.SPDR = command;
	// データ受信フェーズ
    while (RSPI0.SPSR.BIT.IDLNF) ;
	RSPI0.SPCMD0.WORD = 0x2703;				// 転送の最後の時 SSL信号は転送完了で解除
	spi_wait();								// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
    RSPI0.SPDR = 0xFF;
    while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
	command = (unsigned char) (RSPI0.SPDR & 0xFF) ;

	return(command);
}
//[]----------------------------------------------------------------------[]
///	@brief			WRSR(ステータスライト)コマンド(割り込み未使用)
//[]----------------------------------------------------------------------[]
///	@param[in]		val		: シリアルFROMのステータス
///	@return			void
///	@author			A.iiizumi
///	@attention		起動時のFROMリカバリ処理で使用すること(タスク切り替えが始まってからは使用禁止)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteStatus_direct(unsigned char val)
{
	unsigned char command[2] = { 0x01, val };// WRSR, value
	volatile unsigned char dmmy;
	WriteEnable_direct();

// コマンド送信フェーズ
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// 転送の途中の時 SSL信号はアクティブを保持
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
	RSPI0.SPDR = command[0];
	while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
	RSPI0.SPCMD0.WORD = 0x2703;				// 転送の最後の時 SSL信号は転送完了で解除
	RSPI0.SPDR = command[1];
	while (RSPI0.SPSR.BIT.IDLNF) ;
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード

	/* check status to make sure previous operation completed */
	while( FlashReadStatus_direct()&SR_BSY ) {
		dog_reset();
	}

}
//[]----------------------------------------------------------------------[]
///	@brief			イレースコマンド(割り込み未使用)
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: イレース開始アドレス(オールイレースの場合は任意)
///	@param[in]		size	: イレース内容 0:Chip,1:4KB(sector),2:64KB(block)
///	@return			0		: 異常終了
///					1		: 正常終了
///	@author			A.iiizumi
///	@attention		起動時のFROMリカバリ処理で使用すること(タスク切り替えが始まってからは使用禁止)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashErase_direct(const unsigned long address, const unsigned char size)
{
	const unsigned char	erase_commands[] =
						{ 0xc7, 0x20, 0xd8 };// Chip, 4KB, 64KB
	unsigned char	n, command[4];
	int i ;
	volatile unsigned char	dmmy;

	if(size > 2) {
		return (unsigned char)0;			// 要求間違いの場合何もしない
	}

	WriteEnable_direct();
	command[0] = erase_commands[size];
	if( size>0 ) {	// followed address
		n = 4;
		command[1] = (unsigned char)(address >> 16);
		command[2] = (unsigned char)(address >>  8);
		command[3] = (unsigned char)(address >>  0);
	}
	else {
		n = 1;
	}
	// コマンド送信フェーズ
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// 転送の途中の時 SSL信号はアクティブを保持
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
	for (i = 0; i < n; i++) {
		if(i == (n-1)){
			RSPI0.SPCMD0.WORD = 0x2703;		// 転送の最後の時 SSL信号は転送完了で解除
		}
		RSPI0.SPDR = command[i];
		while (RSPI0.SPSR.BIT.IDLNF);
		spi_wait();							// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
		dmmy = (unsigned char) RSPI0.SPDR;	// ダミーリード
	}

	/* check status to make sure previous operation completed */
	while( FlashReadStatus_direct()&SR_BSY ) {
		dog_reset();
	}
	return (unsigned char)1;				// 正常終了
}
//[]----------------------------------------------------------------------[]
///	@brief			ライトコマンド(割り込み未使用)
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: ライト開始開始アドレス
///	@param[in]		*data	: 書き込むデータのポインタ(書き込みサイズは256Byte単位固定) 
///	@return			void
///	@author			A.iiizumi
///	@attention		起動時のFROMリカバリ処理で使用すること(タスク切り替えが始まってからは使用禁止)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteData_direct(unsigned long address, unsigned char const *data)
{
	// 書き込みサイズは256Byte単位の仕様
	unsigned char command[5];
	int i ;
	volatile unsigned char	dmmy;
	unsigned long w_size;

	w_size = 256;
	command[0] = 0x02;						// Byte-Program

	WriteEnable_direct();
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

	while( w_size>0 ){
		dog_reset();
		if(w_size == 1){
			RSPI0.SPCMD0.WORD = 0x2703;		// 転送の最後の時 SSL信号は転送完了で解除
		}
		RSPI0.SPDR = *data;
		while (RSPI0.SPSR.BIT.IDLNF);
		spi_wait();							// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
		dmmy = (unsigned char) RSPI0.SPDR;	// ダミーリード
		/* update address & size values */
		w_size--;
		data++;
	}

	/* check status to make sure previous operation completed */
	while( FlashReadStatus_direct()&SR_BSY ) {
		dog_reset();
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			リードコマンド(割り込み未使用)
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: リード開始開始アドレス
///	@param[out]		*data	: リードデータを書き込むポインタ
///	@param[in]		size	: リードデータサイズ(1～4096(1セクタ))
///	@return			0		: 異常終了
///					1		: 正常終了
///	@author			A.iiizumi
///	@attention		起動時のFROMリカバリ処理で使用すること(タスク切り替えが始まってからは使用禁止)
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

	if((size == 0) || (FROM_SECTOR_SIZE < size)) {
		return (unsigned char)0;			// 範囲外の場合は何もしない
	}

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
		dog_reset();
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
//[]----------------------------------------------------------------------[]
///	@brief			device identification リードコマンド(割り込み未使用)
//[]----------------------------------------------------------------------[]
///	@param[out]		*val	: リードデータを書き込むポインタ(3byte)
///	@return			void
///	@author			A.iiizumi
///	@attention		起動時のFROMリカバリ処理で使用すること(タスク切り替えが始まってからは使用禁止)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashJEDECReadId_direct(unsigned char *val)
{
	unsigned char	command = 0x9F;			// JEDEC-ID
	volatile unsigned char dmmy;
	int i ;

	// コマンド送信フェーズ
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// 転送の途中の時 SSL信号はアクティブを保持
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
	RSPI0.SPDR = command;
	while (RSPI0.SPSR.BIT.IDLNF);
	spi_wait();								// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
	dmmy = (unsigned char) RSPI0.SPDR;		// ダミーリード
	// データ受信フェーズ
	for (i = 0; i < 3; i++) {
		if(i == 2) {
			RSPI0.SPCMD0.WORD = 0x2703;		// 転送の最後の時 SSL信号は転送完了で解除
		}
		RSPI0.SPDR = 0xFF;
		/* Wait for transfer to complete */
		while (RSPI0.SPSR.BIT.IDLNF) ;		// ensure transmit register is empty
		spi_wait();							// IDLNFがクリアされてからwaitをおかないとSPDRの値が読めないため
		val[i] = (unsigned char) (RSPI0.SPDR & 0xFF) ;
	}
}

