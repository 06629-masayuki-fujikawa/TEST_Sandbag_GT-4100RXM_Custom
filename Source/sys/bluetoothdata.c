//[]----------------------------------------------------------------------[]
///	@brief			Bluetooth sci éÛêMäÑÇËçûÇ›èàóùèàóù
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/06<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#include "iodefine.h"
#include "system.h"	
#include "bluetooth.h"

t_BtSciRcvCtrl	Bt_RcvCtrl;
t_BtSciSndCtrl	Bt_SndCtrl;
uchar			Bt_RcvBuf[ BT_SCI_RCVBUF_SIZE ];
uchar			Bt_SndBuf[ BT_SCI_SNDBUF_SIZE ];
uchar			Btcomdr_RcvData[BT_SCI_RCVBUF_SIZE];
ushort			Btcom_Timer_1_Value;									// ï∂éöä‘
const char bt_init_cmd1[]={"AT+WRSEC=0,1,0\r\n"};
const char bt_init_cmd2[]={"AT+WRISPS=1000,0012,0012,0011\r\n"};
const char bt_ok[] ={"\r\nOK\r\n"};

