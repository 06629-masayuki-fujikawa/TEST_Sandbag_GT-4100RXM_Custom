#include "iodefine.h"
#include "system.h"	
#include "bluetooth.h"

//#include "fbcom.h"


void	Bluetooth_Int_RXI( void );
void	Bluetooth_Int_ERI( void );
uchar	Bluetoothsci_SndReq( ushort );
uchar	Bluetoothsci_IsSndCmp( void );
void	Bluetooth_Int_TXI( void );
void	Bluetooth_Int_TEI( void );

//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth sci 受信割り込み処理処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Bluetooth_Int_RXI( void )
{
	volatile uchar   rcvdat;
	/************************************************************/
	/*	RX630のSSRにはRDRFがないので							*/
	/*	ORER(Overrun) FER(Framing) PER(Parity)のErrorがなければ	*/
	/*	ReadしてReadDataを格納する(RDRF <-0もなし)				*/
	/************************************************************/
	if((SCI6.SSR.BIT.ORER==0) && (SCI6.SSR.BIT.FER==0) && (SCI6.SSR.BIT.PER==0)) {
		rcvdat = SCI6.RDR;

Bluetooth_Sci_Rcv_10:
		if( BT_SCI_RCVBUF_SIZE > Bt_RcvCtrl.RcvCnt ){									// receive buffer full (N)
			Bt_RcvBuf[ Bt_RcvCtrl.WriteIndex ] = rcvdat;								// received data set
			++Bt_RcvCtrl.RcvCnt;														// counter up

			if( Bt_RcvCtrl.RcvCnt == 1 ){												// 1文字目受信
//				Btcom_SetReceiveData();
//				Btcom_20mTimStart2( Btcom_Timer_2_Value );								// 文字受信監視開始
//				Btcom_20mTimStop();
			}

			/* write index update */
			if( (BT_SCI_RCVBUF_SIZE - 1) > Bt_RcvCtrl.WriteIndex ){
				++Bt_RcvCtrl.WriteIndex;
			}
			else {
				Bt_RcvCtrl.WriteIndex = 0;
			}
		}
		else {		/* receive buffer overflow count update */
			Bt_RcvCtrl.OvfCount = 1;
		}
		SCI6.SSR.BYTE = 0;																// 割り込み要因のクリア
		dummy_Read = SCI6.SSR.BYTE;														// Dummy Read
		if( Bt_RcvCtrl.OvfCount != 1 ){
			Btcom_1mTimStart( Btcom_Timer_1_Value );									// 文字間ﾀｲﾏｰ起動
		}
	}
	else{																				// RDRF=0, ORER=1, FER=0, PER=0
		rcvdat = 0xff;
		goto Bluetooth_Sci_Rcv_10;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth sci エラー割り込み処理処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Bluetooth_Int_ERI(void)
{
	volatile unsigned char	c;

	if( SCI6.SSR.BIT.ORER )																// over run error ?
	{
		Bt_RcvCtrl.ComerrStatus |= 0x01;												// over run set
	}
	if( SCI6.SSR.BIT.FER )																// frame error ?
	{
		Bt_RcvCtrl.ComerrStatus |= 0x02;												// frame error set
	}
	if( SCI6.SSR.BIT.PER )																// parity error ?
	{
		Bt_RcvCtrl.ComerrStatus |= 0x04;												// parity error set
	}
	c = SCI6.RDR;																		// Received Data
	SCI6.SSR.BIT.ORER = 0 ;																// ERROR clear
	SCI6.SSR.BIT.FER = 0 ;
	SCI6.SSR.BIT.PER = 0 ;
	SCI6.SSR.BYTE &= 0xc7;
	dummy_Read = SCI6.SSR.BYTE;															// Dummy Read
}

//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth sci 送信要求処理
//[]----------------------------------------------------------------------[]
///	@param[in]		Length	: 送信データレングス
///	@return			1		: 異常終了
///					0		: 正常終了
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	Bluetoothsci_SndReq( ushort Length )
{

	SCI6.SCR.BIT.TE = 1;
	if( (SCI6.SSR.BIT.ORER==0) && (SCI6.SSR.BIT.FER==0) && (SCI6.SSR.BIT.PER==0) ){		// send enable (Y)
		Bt_SndCtrl.SndCmpFlg = 0;														// send complete flag clear(OFF)

		Bt_SndCtrl.SndReqCnt = Length;
		Bt_SndCtrl.SndCmpCnt = 0;
		Bt_SndCtrl.ReadIndex = 1;

		SCI6.TDR = Bt_SndBuf[0];														// send charcter set
		SCI6.SSR.BIT.ORER= 0;															// send start
		SCI6.SSR.BIT.FER= 0;
		SCI6.SSR.BIT.PER= 0;
		dummy_Read = SCI6.SSR.BYTE;														// Dummy Read

		SCI6.SCR.BIT.TIE = 1;															// interrupt enable
		return (unsigned char)0;
	}
	else {																				// send invalid
		Bt_SndCtrl.SndCmpFlg = 1;														// send complete flag set (ON)	
		SCI6.SCR.BIT.TEIE = 0;															// interrupt enable (TEI)
		SCI6.SCR.BIT.TIE = 0;															// send interrupt disable
		dummy_Read = SCI6.SSR.BYTE;														// Dummy Read
		return (unsigned char)1;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth sci 送信完了判定処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	Bluetooth_IsSndCmp( void )
{
	if( 1 == Bt_SndCtrl.SndCmpFlg ){
		return (uchar)1;
	}
	return (uchar)0;
}
//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth sci TXI割り込み処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Bluetooth_Int_TXI( void )
{

	if((SCI6.SSR.BIT.ORER==0) && (SCI6.SSR.BIT.FER==0) && (SCI6.SSR.BIT.PER==0)){		// send enable (Y)
		++Bt_SndCtrl.SndCmpCnt;															// send completed count up
		if( Bt_SndCtrl.SndReqCnt > Bt_SndCtrl.SndCmpCnt ){								// not end yet (Y)
			SCI6.TDR = Bt_SndBuf[ Bt_SndCtrl.ReadIndex++];								// send charcter set
			SCI6.SSR.BIT.ORER= 0;														// send start
			SCI6.SSR.BIT.FER= 0;
			SCI6.SSR.BIT.PER= 0;
			dummy_Read = SCI6.SSR.BYTE;													// Dummy Read

			SCI6.SCR.BIT.TIE = 1;														// interrupt enable
		}
		else {																			// all data send end
			SCI6.SCR.BIT.TIE = 0;														// send enable interrupt inhibit
			SCI6.SCR.BIT.TEIE = 1;
			Bt_SndCtrl.SndCmpFlg = 1;													// send complete flag set
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth sci TEI割り込み処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Bluetooth_Int_TEI( void )
{
	// I don't use this event
	SCI6.SCR.BIT.TE = 0;
	SCI6.SCR.BIT.TEIE = 0;
}

