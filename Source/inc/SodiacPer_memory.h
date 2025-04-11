/*************************************************************************/ /**
 *	\file	SodiacPer_memory.h
 *	\brief	Sodiac ペリフェラル メモリ
 *	\author	Copyright(C) Arex Co. 2008-2011 All right Reserved.
 *	\author	株式会社アレックス 2008-2011
 *	\date	CREATE : 17/Nov/2008
 *	\date	2011.03.02 Iwasawa H. sodiac_read_end 関数の追加
 *	\date	2011/04/20 Ver.2.30
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#include "SodiacAPI.h"

#ifndef	_defSodiacPer_memory_H_
#define	_defSodiacPer_memory_H_

/* 音声データ読み込み関数プロトタイプ */
/*************************************************************************/ /**
 *	メモリ読込み関数
 *
 *	指定されたアドレスのデータを1byte読み込む
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\retval		D_SODIAC_E_OK : 成功
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern unsigned char	sodiac_dataread(int iChannel,unsigned long ulAddress);
/*************************************************************************/ /**
 *	メモリ読込み関数
 *
 *	指定された番地から指定されたサイズだけデータを読み込む
 *	\param[out]	vpPt      : データ格納バッファ
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\param[in]	iSize     : データ読み出しサイズ
 *	\retval		D_SODIAC_E_OK : 成功
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern D_SODIAC_E		sodiac_nread(int iChannel,void *vpPt, unsigned long ulAddress, int iSize);

/* ヘッダ読み込み関数プロトタイプ */
/*************************************************************************/ /**
 *	メモリ読込み関数
 *
 *	指定されたアドレスのデータを1byte読み込む
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\retval		D_SODIAC_E_OK : 成功
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern unsigned char	sodiac_headerread(unsigned long ulAddress);

/*************************************************************************/ /**
 *	メモリ読込み関数
 *
 *	指定された番地から指定されたサイズだけデータを読み込む
 *	\param[out]	vpPt      : データ格納バッファ
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\param[in]	iSize     : データ読み出しサイズ
 *	\retval		D_SODIAC_E_OK : 成功
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 **/ /*************************************************************************/
extern D_SODIAC_E		sodiac_hnread(void *vpPt, unsigned long ulAddress, int iSize);

#endif	/* _defSodiacPer_memory_H_ */

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2011		  *
 **/ /*************************************************************************/
