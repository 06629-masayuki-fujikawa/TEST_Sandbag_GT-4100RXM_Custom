// GG120600(S) // DT-10 テストプログラム(S)
#include "iodefine.h"
#include "system.h"

#define	DT_INLINE	static
#define	DT_UINT		unsigned int

void Rspi_DT10_Init(void);
void DT10_Port_init(void);

void Rspi_DT10_Init(void)
{
	// RSPI1の初期設定
	MSTP(RSPI1) = 0;
	RSPI1.SSLP.BYTE = 0x00;		// SSL0P～SSL3PはLアクティブ(デフォルト)
	//RSPI1.SPPCR.BYTE = 0x00;	// (デフォルト)
	RSPI1.SPPCR.BYTE = 0x10;	// (デフォルト)
	RSPI1.SPSCR.BYTE = 0x00;	// (デフォルト)
	
	/* 通信速度を選択してください */
	//RSPI1.SPBR.BYTE = 0x02;		// 8.33Mbps
	RSPI1.SPBR.BYTE = 0x01;		// 12.5Mbps
	//RSPI1.SPBR.BYTE = 0x00;		// 25Mbps
	
	
	RSPI1.SPDCR.BYTE = 0x30;	// 32-bit data, 1 frame
	RSPI1.SPCKD.BYTE = 0x00;	// (デフォルト)
	RSPI1.SSLND.BYTE = 0x00;	// (デフォルト)
	RSPI1.SPND.BYTE = 0x00;		// (デフォルト)
	RSPI1.SPCR2.BYTE = 0x00;	// パリティ付加なし
	// SPCMDのSSL信号保ビットは転送するデータ長によって変更する必要があります。(初期値は転送終了でネゲートとする)
	RSPI1.SPCMD0.WORD = 0x2103;	// b15:SCKDEN=0(RSPCK遅延は1RSPCLK)
								// b14:SLNDEN=0(SSLネゲート遅延は1RSPCK)
								// b13:SPNDEN=1(次アクセス遅延はRSPI次アクセス遅延レジスタ（SPND）の設定値)
								// b12:LSBF=0(MSBファースト)
								// b11-8:SPB=0001(24bit)
								// b7:SSL信号レベル保持ビットSSLKP=0(転送終了時に全SSL信号をネゲート)
								// b6-4:SSLA=000(SSL0)
								// b3-2:BRDV=00(ベースの分周比 12.5MHz)
								// b1:CPOL=1(アイドル時のRSPCKが“1”)
								// b0:CHPA=1(奇数エッジでデータ変化、偶数エッジでデータサンプル)
								// モード3
	DT10_Port_init();
	RSPI1.SPCR.BYTE = 0x4A;		// Enable RSPI in master mode
}

void DT10_Port_init(void)
{

	MPC.PWPR.BIT.B0WI = 0;                  /* enable writing PFSWE bit */
    MPC.PWPR.BIT.PFSWE = 1;                 /* enable writing PFS register */

	MPC.P26PFS.BYTE = 0x0D;                 // b7-b5, PSEL:5 MISOB-B
	MPC.P27PFS.BYTE = 0x0D;                 // b7-b5, PSEL:5 RSPCKB
	MPC.P30PFS.BYTE = 0x0D;                 // b7, ISEL:1, b5, PSEL:5 MISOB
	MPC.P31PFS.BYTE = 0x0D;                 // b7, ISEL:1, b5, PSEL:5 SSLB0

    MPC.PWPR.BIT.PFSWE = 0;                 /* disable writing PFS register */
    MPC.PWPR.BIT.B0WI = 1;                  /* disable writing PFSWE bit */

	/********************************************/
	/*	Port2									*/
	/*		P20	SDA1	I		SDA				*/
	/*		P21	SCL1	I		SCL				*/
	/*		P22	P22		O(H)	#FB_MD2			*/
	/*		P23	TXD3	O		#FB_TXD			*/
	/*		P24	P24		O(L)	FB_FW			*/
	/*		P25	RXD3	I		#FB_RXD			*/
	/*		P26	TDO		I→O	TDO→MOSIB		*/
	/*		P27	TCK		I→O	TCK→RSPCKB		*/
	/********************************************/
	PORT2.PMR.BYTE  = 0xEB;		//	B7(1), B6(1), B5(1), B4(0), B3(1), B2(0), B1(1), B0(1)
	PORT2.PDR.BYTE  = 0xDC;		//	B7(1), B6(1), B5(0), B4(1), B3(1), B2(1), B1(0), B0(0)
	/********************************************/
	/*	Port3									*/
	/*		P30	TDI		I		TDI→MISOB		*/
	/*		P31	TMS		I→O	TMS→SSLBO		*/
	/*		P32	P32		O(H)	#FB_RES			*/
	/*		P33	TIOCD0	O		SOUND			*/
	/*		P34	#TRST	I		＃TRST			*/
	/*		P35	NMI		I		#NMI			*/
	/*		P36	EXTAL	0		EXTAL(入力扱いにすること)	*/
	/*		P37	XTAL	O		XTAL(入力扱いにすること)	*/
	/********************************************/
	PORT3.PMR.BYTE  = 0x0B;		//	B7(0), B6(0), B5(0), B4(0), B3(1), B2(0), B1(1), B0(1)
	PORT3.PDR.BYTE  = 0x0E;		//	B7(0), B6(0), B5(0), B4(0), B3(1), B2(1), B1(1), B0(0)

}


//------------------------------------------------------------------------------
//	Func:	enterCritical
//	Desc:	Please describe the code to enter Critical Section.
//------------------------------------------------------------------------------
DT_INLINE void enterCritical(void)
{
	_di();
}

//------------------------------------------------------------------------------
//	Func:	exitCritical
//	Desc:	Please describe the code to exit Critical Section.
//------------------------------------------------------------------------------
DT_INLINE void exitCritical(void)
{
	_ei();
}

//==============================================================================
//	Desc:	Test Point ID
//==============================================================================
#define		DT_VARIABLE_BIT			0x02
#define		DT_EVTTRG_BIT			0x08
#define		DT_VARIABLE_FAST_BIT	0x01
#define		DT_EVTTRG_FAST_BIT		0x02

//------------------------------------------------------------------------------
//	Func: _TP_Bus1BitOutDrv
//	Desc: 1bit Data Output Function
//------------------------------------------------------------------------------
DT_INLINE void _TP_Bus1BitOutDrv( DT_UINT dat )
{
	while (RSPI1.SPSR.BIT.IDLNF);
	RSPI1.SPDR = dat;
	while (RSPI1.SPSR.BIT.IDLNF);
}

//------------------------------------------------------------------------------
//	Func: _TP_BusOutDrv
//	Desc: Test Point Output Function
//------------------------------------------------------------------------------
DT_INLINE void _TP_BusOutDrv( DT_UINT addr, DT_UINT dat )
{
		RSPI1.SPCMD0.WORD = 0x2F83; // 16bit 開始
		_TP_Bus1BitOutDrv( dat );
	if( addr != 0 ){
		RSPI1.SPCMD0.WORD = 0x2103; // 24bit 終端データ
		_TP_Bus1BitOutDrv( addr );
	}
}

//------------------------------------------------------------------------------
//	Func:	_TP_BusOutByteDrv
//	Desc:	Byte Data Output Function
//------------------------------------------------------------------------------
DT_INLINE void _TP_BusOutByteDrv( DT_UINT dat )
{
	_TP_Bus1BitOutDrv(dat);
}

//------------------------------------------------------------------------------
//	Func:	_TP_BusOutFastDrv
//	Desc:	Test Point Output(Fast) Function
//------------------------------------------------------------------------------
DT_INLINE void _TP_BusOutFastDrv( DT_UINT step, DT_UINT bit )
{
	RSPI1.SPCMD0.WORD = 0x2703; // 8bit
	_TP_Bus1BitOutDrv( step );
}

//------------------------------------------------------------------------------
//	Func:	_TP_MemoryOutDrv
//	Desc:	Value Output Function
//------------------------------------------------------------------------------
DT_INLINE void _TP_MemoryOutDrv( unsigned char *p, DT_UINT size )
{
	if( size >= 256 ) size = 256;
	RSPI1.SPCMD0.WORD = 0x2783; // 8bit 開始
	_TP_BusOutByteDrv( size );
	
		
	for( ; size != 0; --size, ++p ){
		if(size == 1){
			RSPI1.SPCMD0.WORD = 0x2703; // 8bit 終端データ
		}
		
		_TP_BusOutByteDrv( *p );
	}
}

//------------------------------------------------------------------------------
//	Func: _TP_EventOutDrv
//	Desc: Event Output Function
//------------------------------------------------------------------------------
DT_INLINE void _TP_EventOutDrv( DT_UINT dat )
{
	RSPI1.SPCMD0.WORD = 0x2783; // 8bit 開始
	_TP_BusOutByteDrv( (dat) );
	_TP_BusOutByteDrv( (dat) >> 8 );
	_TP_BusOutByteDrv( 0x00 );
	RSPI1.SPCMD0.WORD = 0x2703; // 8bit 終端データ
	_TP_BusOutByteDrv( 0x00 );
}

//------------------------------------------------------------------------------
//	Func: _TP_BusOut
//	Desc: Called by Test Point
//------------------------------------------------------------------------------
void _TP_BusOut( DT_UINT addr, DT_UINT dat )
{
	enterCritical();
	_TP_BusOutDrv( addr, dat );
	exitCritical();
}

//------------------------------------------------------------------------------
//	Func: _TP_MemoryOutput
//	Desc: Called by Variable Test Point
//------------------------------------------------------------------------------
void _TP_MemoryOutput( DT_UINT addr, DT_UINT dat, void *value, DT_UINT size )
{
	enterCritical();
	_TP_BusOutDrv( addr | DT_VARIABLE_BIT, dat );
	_TP_MemoryOutDrv( (unsigned char *)value, size );
	exitCritical();
}

//------------------------------------------------------------------------------
//	Func: _TP_EventTriggerFast
//	Desc: Called by Event Trigger Test Point
//------------------------------------------------------------------------------
void _TP_EventTrigger( DT_UINT addr, DT_UINT dat, DT_UINT event_id )
{
	enterCritical();
	_TP_BusOutDrv( addr | DT_EVTTRG_BIT, ((unsigned short)(dat&0x0FFF))<<4 );
	exitCritical();
}

//------------------------------------------------------------------------------
//	Func: _TP_BusOutFast
//	Desc: Called by Fast Test Point
//------------------------------------------------------------------------------
void _TP_BusOutFast( DT_UINT step, DT_UINT bit )
{
	step = step << 2;
	bit = bit + 2;
	enterCritical();
	_TP_BusOutFastDrv( step, bit );
	exitCritical();
}

//------------------------------------------------------------------------------
//	Func: _TP_MemoryOutputFast
//	Desc: Called by Variable Fast Test Point
//------------------------------------------------------------------------------
void _TP_MemoryOutputFast( DT_UINT step, DT_UINT bit, void *value, DT_UINT size )
{
	step = (step << 2) + DT_VARIABLE_FAST_BIT;
	bit = bit + 2;
	enterCritical();
	_TP_BusOutFastDrv( step, bit );
	_TP_MemoryOutDrv( (unsigned char *)value, size );
	exitCritical();
}

//------------------------------------------------------------------------------
//	Func: _TP_EventTriggerFast
//	Desc: Called by Event Trigger Fast Test Point
//------------------------------------------------------------------------------
void _TP_EventTriggerFast( DT_UINT dat )
{
	enterCritical();
	_TP_BusOutFastDrv( DT_EVTTRG_FAST_BIT, 2 );
	_TP_EventOutDrv( dat );
	exitCritical();
}

// =============================================================================
// GG120600(E) // DT-10 テストプログラム(E)
