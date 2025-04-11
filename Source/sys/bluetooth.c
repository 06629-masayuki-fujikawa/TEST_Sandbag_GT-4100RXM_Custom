#include	"system.h"
#include	"prm_tbl.h"
#include	"bluetooth.h"
#include	"stdio.h"
#include <string.h>

void	Bluetooth_unit(void);
uchar	Btcom_EventCheck( void );
void	Btcomdr_RcvInit( void );
void	Btcomdr_SendInit( void );
void	Btcom_Init( void );

void	Btcom_1mTimStart( ushort );
void	Btcom_1mTimStop( void );
uchar	Btcom_1mTimeout( void );

void	Btcom_20mTimStart( ushort );
void	Btcom_20mTimStop( void );
uchar	Btcom_20mTimeout( void );


//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能メイン処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Bluetooth_unit(void)
{
	unsigned char event;
	unsigned short length;
	static unsigned char state = 0;
	int i;

	if( 1 == prm_get(COM_PRM, S_PAY, 26, 1, 1) ){// 02-0026⑥=1 Bluetooth接続有
		if(f_bluetooth_init < 9) {
			//bluetoothユニット初期化処理
			switch(f_bluetooth_init) {
				case 0:
					PORTJ.PODR.BIT.B3 = 0;									// BT_RTS=0	
					/************************/
					/*	Command Modeで起動	*/
					/************************/
					PORT9.PODR.BIT.B1 = 0;									// BT_MODE=1(Command Mode)
					PORTF.PODR.BIT.B5 = 1;									// BT_RES=1(リセット)
					Btcom_20mTimStart(PORT_WAIT);							// 信号変化後安定待ち
					f_bluetooth_init++;
					// 5ms必要
					break;
				case 1:
					if(Btcom_20mTimeout() != 0) {							// タイムアウト
						PORTF.PODR.BIT.B5 = 0;								// BT_RES=0(リセット解除)
						Btcom_20mTimStart(PON_TIMEOUT);						// PON通知待ち
						f_bluetooth_init++;
					}
					break;
				case 2:
					event = Btcom_EventCheck();
					if(1 == event){											// 受信データ有
						// 最初は"OK"を受信するので読み捨てする
						/********************************/
						/*	SCI6送信 AT+WRSEC=0,1,0		*/
						/********************************/
						Btcom_20mTimStop();									// レスポンス待ちタイマー停止
						length = strlen(bt_init_cmd1);
						memcpy(Bt_SndBuf,bt_init_cmd1,length);				// 送信データセット
						Bluetoothsci_SndReq( length );						// 送信開始
						Btcom_20mTimStart(RSP_TIMEOUT);						// レスポンスタイマーセット
						f_bluetooth_init++;
					}else if(2 == event){									// 受信タイムアウト
						Btcom_20mTimStop();									// レスポンス待ちタイマー停止
						f_bluetooth_init = 0;								// 無応答のため最初からやり直し
					}
					break;
				case 3:
					event = Btcom_EventCheck();
					if(1 == event){											// 受信データ有
						Btcom_20mTimStop();									// レスポンス待ちタイマー停止
						length = strlen(bt_ok);
	    				if( memcmp( &Btcomdr_RcvData[0], &bt_ok[0], length ) == 0 ) {
							f_bluetooth_init++;								// ACK受信のため次へ
						} else {
							f_bluetooth_init = 0;							// 最初からやり直し
						}
					}else if(2 == event){									// 受信タイムアウト
						Btcom_20mTimStop();									// レスポンス待ちタイマー停止
						f_bluetooth_init = 0;								// 無応答のため最初からやり直し
					}
					break;
				case 4:
					/********************************************/
					/*	SCI6送信 AT+WRISPS=1000,0012,0012,0011	*/
					/********************************************/
					length = strlen(bt_init_cmd2);
					memcpy(Bt_SndBuf,bt_init_cmd2,length);
					Bluetoothsci_SndReq( BT_INIT_CMD_2_LEN );
					f_bluetooth_init++;
					break;
				case 5:
					event = Btcom_EventCheck();
					if(1 == event){											// 受信データ有
						Btcom_20mTimStop();									// レスポンス待ちタイマー停止
						length = strlen(bt_ok);
			 			if( memcmp( &Btcomdr_RcvData[0], &bt_ok[0], length ) == 0 ) {
							f_bluetooth_init++;								// ACK受信のため次へ
						} else {
							f_bluetooth_init = 0;							// 最初からやり直し
						}
					}else if(2 == event){									// 受信タイムアウト
						Btcom_20mTimStop();									// レスポンス待ちタイマー停止
						f_bluetooth_init = 0;								// 無応答のため最初からやり直し
					}
					break;
				case 6:
					/****************************/
					/*	Automatic Modeに変更	*/
					/****************************/
					PORT9.PODR.BIT.B1 = 1;									// BT_MODE=1(Automatic Mode)
					f_bluetooth_init++;
					break;
				case 7:
					PORTF.PODR.BIT.B5 = 1;									// BT_RES=1(リセット)
					Btcom_20mTimStart(PORT_WAIT);							// 信号変化後安定待ち
					f_bluetooth_init++;
					// 5ms必要
					break;
				case 8:
					if(Btcom_20mTimeout() != 0) {							// タイムアウト
						Btcom_20mTimStop();									// レスポンス待ちタイマー停止
						PORTF.PODR.BIT.B5 = 0;								// BT_RES=0(リセット解除)
						f_bluetooth_init++;
					}
					break;
				default:
					break;
			}
		}else {
			//データ受信待ち
			event = Btcom_EventCheck();
			if(1 == event){											// 受信データ有
				switch(state) {
					case 0:
						if((Btcomdr_RcvData[0] == 't')||( Btcomdr_RcvData[0] == 'T')) {// 温度コマンド
							state = 1;// CRLF待ち
						}
						break;
					case 1:
			 			if(Btcomdr_RcvData[0] == '\r') {
							length = 0;
							for(i = 0;i < FAN_EXE_BUF;i++) {
								if(fan_exe_time.fan_exe[i].f_exe == 1){// ON
									length += (unsigned short)sprintf((char*)&Bt_SndBuf[length],"ON  %04d:%02d:%02d:%02d:%02d\r\n",
																		fan_exe_time.fan_exe[i].year,
																		fan_exe_time.fan_exe[i].mont,
																		fan_exe_time.fan_exe[i].date,
																		fan_exe_time.fan_exe[i].hour,
																		fan_exe_time.fan_exe[i].minu);
								} else if(fan_exe_time.fan_exe[i].f_exe == 2){//OFF
									length += (unsigned short)sprintf((char*)&Bt_SndBuf[length],"OFF %04d:%02d:%02d:%02d:%02d\r\n",
																		fan_exe_time.fan_exe[i].year,
																		fan_exe_time.fan_exe[i].mont,
																		fan_exe_time.fan_exe[i].date,
																		fan_exe_time.fan_exe[i].hour,
																		fan_exe_time.fan_exe[i].minu);
								}
							}
							if(length == 0){
								length = (unsigned short)sprintf((char*)Bt_SndBuf,"データなし\r\n");
							}
							Bluetoothsci_SndReq( length );
						}
						state = 0;// idleに戻す
						break;
					default:
						break;
				}
			}
		}
	}
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			イベントチェック処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	Btcom_EventCheck( void )
{
	if( Btcom_1mTimeout() != 0 ){												// 文字間タイムアウト発生(受信データあり)
		memset(Btcomdr_RcvData,0x00,sizeof(Btcomdr_RcvData));
		_di();
		memcpy(Btcomdr_RcvData,&Bt_RcvBuf[Bt_RcvCtrl.ReadIndex],Bt_RcvCtrl.RcvCnt);// 受信データを取得
		_ei();
		Btcomdr_RcvInit();// 受信管理領域を初期化
		return(1);// データ受信
	}
	if( Btcom_20mTimeout() != 0 ){												// レスポンスタイムアウト
		return (uchar)2;
	}
	return (uchar)0;// イベント無し
}

//[]----------------------------------------------------------------------[]
///	@brief			受信管理エリア初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Btcomdr_RcvInit( void )
{
	Bt_RcvCtrl.RcvCnt		= 0;
	Bt_RcvCtrl.ReadIndex	= 0;
	Bt_RcvCtrl.WriteIndex	= 0;
	Bt_RcvCtrl.OvfCount		= 0;
	Bt_RcvCtrl.ComerrStatus	= 0;
	Btcom_1mTimStop();
}
//[]----------------------------------------------------------------------[]
///	@brief			送信管理エリア初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Btcomdr_SendInit( void )
{
	Bt_SndCtrl.SndCmpFlg = 0;
	Bt_SndCtrl.SndReqCnt = 0;
	Bt_SndCtrl.SndCmpCnt = 0;
	Bt_SndCtrl.ReadIndex = 0;
}
//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Btcom_Init( void )
{
	Sci6Init();
	Btcom_20mTimStop();
	Btcomdr_RcvInit();
	Btcomdr_SendInit();
	Btcom_Timer_1_Value = 1;								// 文字間タイムアウト 1ms
	memset(Bt_RcvBuf,0x00,sizeof(Bt_RcvBuf));
	memset(Bt_SndBuf,0x00,sizeof(Bt_SndBuf));
	f_bluetooth_init = 0;									// bluetoothイニシャルシーケンス制御フラグ
}
//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Btcom_1mTimStart( ushort TimVal )
{
	Btcom_1msT1 = TimVal;
}
//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Btcom_1mTimStop( void )
{
	Btcom_1msT1 = 0x8000;
}

//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	Btcom_1mTimeout( void )
{
	if( 0 == Btcom_1msT1 ){
		return	(uchar)1;
	}
	return	(uchar)0;
}

//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Btcom_20mTimStart( ushort TimVal )
{
	if( TimVal != 0 ){
		Btcom_20msT1 = TimVal;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Btcom_20mTimStop( void )
{
	Btcom_20msT1 = 0x8000;
}

//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth機能
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	Btcom_20mTimeout( void )
{
	if( 0 == Btcom_20msT1 ){
		return	(uchar)1;
	}
	return	(uchar)0;
}
