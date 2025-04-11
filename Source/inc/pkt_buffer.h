//[]----------------------------------------------------------------------[]
///	@brief			パケット通信バッファ管理モジュール関数定義
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pkt_buffer.h
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#ifndef	__PKT_BUFFER_H__
#define	__PKT_BUFFER_H__

//void	PKTbuf_init( char buf_type );
//BOOL	PKTbuf_SetReSendCommand( char buf_type, const uchar *data, ushort len );
//ushort	PKTbuf_ReadReSendCommand( char buf_type, ushort num, uchar *data );
void	PKTbuf_init( void );
BOOL	PKTbuf_SetReSendCommand( const uchar *data, ushort len );
ushort	PKTbuf_ReadReSendCommand( ushort num, uchar *data );
BOOL	PKTbuf_SetSendCommand( const uchar *data, ushort len );
BOOL	PKTbuf_SetSendCommand_Client( const uchar *data, ushort len );		// 複数クライアント用
BOOL	PKTbuf_SetSendCommand_internal( const uchar *data, ushort len );	// 実通信処理
ushort	PKTbuf_ReadSendCommand( uchar *data, ushort limit );
BOOL	PKTbuf_SetRecvCommand( const uchar *data, ushort len );
ushort	PKTbuf_ReadRecvCommand( uchar *data );
ulong	PKTbuf_SetSendCommandSync( const uchar *data, ushort len,ushort nLane );
//void	PKTbuf_ClearReSendCommand( char buf_type );
void	PKTbuf_ClearReSendCommand( void );
ulong	PKTbuf_CheckReSendCommand( void );

#endif	// __PKT_BUFFER_H__
