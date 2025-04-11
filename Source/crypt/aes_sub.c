#include	<string.h>
#include	<stdlib.h>
#include "system.h"
#include "mem_def.h"
#include "prm_tbl.h"
#include "aes_sub.h"

#if defined( AES_ENC_PREKEYED ) || defined( AES_DEC_PREKEYED )

static const uchar *key[] = {
	"AMANO_PARKINGWEB",				// ID = CRYPT_KEY_STANDARD
	"AMANO_REMOTEDOWN",				// ID = CRYPT_KEY_REMOTEDL
// MH810100(S) K.Onodera 2019/11/20 車番チケットレス (RT精算データ対応)
	"AMANO_RXMLCD_PKT",				// ID = CRYPT_KEY_RXMLCDPK
// MH810100(E) K.Onodera 2019/11/20 車番チケットレス (RT精算データ対応)
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	"6HIXZPEWSPQ3MH9E"				// ID = CRYPT_KEY_PAYEDQR
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	,"X4V5G3J7RTFKYB6Q"				// ID = CRYPT_KEY_QRRECEIPT
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
};

// 初期化ベクタ
static const uchar	CrpIV[][N_BLOCK] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},// ID = CRYPT_KEY_STANDARD
	{0x34, 0xa2, 0xbb, 0x07, 0xd0, 0x51, 0xf8, 0x52,0x3a, 0xaa, 0x2f, 0x61, 0x00, 0x29, 0x1c, 0x88},// ID = CRYPT_KEY_REMOTEDL
// MH810100(S) K.Onodera 2019/11/20 車番チケットレス (RT精算データ対応)
	{0x22, 0x42, 0xfe, 0x29, 0x12, 0x51, 0x80, 0x71,0x4b, 0xbb, 0xc7, 0x45, 0x00, 0x27, 0x81, 0x5c},// ID = CRYPT_KEY_RXMLCDPK
// MH810100(E) K.Onodera 2019/11/20 車番チケットレス (RT精算データ対応)
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},// ID = CRYPT_KEY_PAYEDQR
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},// ID = CRYPT_KEY_QRRECEIPT
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
};
unsigned char iv[ N_BLOCK ], indata[ DATA_SIZE ];
aes_context ctx[1];

#endif

#if defined( AES_ENC_PREKEYED )

void AesCBCEncrypt( unsigned char *outdata, unsigned short size )
{
	ushort	key_idx = 0;	// 暗号化キーの指定インデックス（現在は０固定）

	if( !size ){
		return;
	}

	// 暗号化キーが設定されていない場合は仮の初期値をセットする
	if( Encryption_Key[key_idx][0] == 0 ){
		memcpy( Encryption_Key[key_idx], key[0], 16 );
	}
	else{													// else節の処理は橋本さんの指摘により追加
		memcpy( Encryption_Key[key_idx], key[0], 16 );
	}

	// AES CBCモード
	aes_set_key( Encryption_Key[key_idx], 16, ctx );

	memcpy( indata, outdata, size );
	memset( iv, 0x00, N_BLOCK );	// 初期ベクタ0
	aes_cbc_encrypt( indata, outdata, (size/N_BLOCK), iv, ctx );
}
// MH810100(S) K.Onodera 2019/11/20 車番チケットレス (RT精算データ対応)
int EncryptWithKeyAndIV_NoPadding( int KeyAndIVIndex, unsigned char * pIndata,unsigned char *pOutdata, int outSize )
{
	uchar	ucIV[N_BLOCK];

	if( !outSize ){
		return 0;
	}

	// AES CBCモード
	aes_set_key( key[KeyAndIVIndex], CRYPT_KEY_LENGTH, ctx );

	memset( pOutdata, 0x00, outSize );
	memcpy( ucIV,CrpIV[KeyAndIVIndex],sizeof(ucIV) );	// 渡すIVは書き換わってしまうので変数へCOPY
	
	if( aes_cbc_encrypt( pIndata, pOutdata, (outSize/N_BLOCK), ucIV, ctx ) != EXIT_SUCCESS){
		return 0;
	}else{
		return outSize;
	}
}
// MH810100(E) K.Onodera 2019/11/20 車番チケットレス (RT精算データ対応)

// ※16バイト余裕をもったバッファにすること（チェック用に使用されるため）
int EncryptWithKeyAndIV(int KeyAndIVIndex,unsigned char * pIndata,unsigned char *pOutdata, int inSize,int outSize )
{
	int renewsize = 0;
	int i = 0;
	uchar ucIV[N_BLOCK];
	
	renewsize = ((inSize - 1) / N_BLOCK + 1) * N_BLOCK;
	if (renewsize == inSize)
	{
		renewsize += N_BLOCK;
	}
	if (renewsize > outSize || KeyAndIVIndex >= CRYPT_KEY_MAX)
	{
		return 0;
	}
	for (i = inSize; i < renewsize; i++)
	{
		pIndata[i] = (unsigned char)(renewsize - inSize);
	}
	// Keyセット
	aes_set_key( key[KeyAndIVIndex], CRYPT_KEY_LENGTH, ctx );
	
	memset( pOutdata, 0x00, outSize );
	memcpy(ucIV,CrpIV[KeyAndIVIndex],sizeof(ucIV));		// 渡すIVは書き換わってしまうので変数へCOPY
	
	if( aes_cbc_encrypt( pIndata, pOutdata, (renewsize/N_BLOCK), ucIV, ctx ) != EXIT_SUCCESS){
		return 0;
	}
	
	return renewsize;
}

#endif

#if defined( AES_DEC_PREKEYED )

void AesCBCDecrypt( unsigned char *outdata, unsigned short size )
{
	ushort	key_idx = 0;	// 暗号化キーの指定インデックス（現在は０固定）

	if( !size ){
		return;
	}

	// 暗号化キーが設定されていない場合は仮の初期値をセットする
	if( Encryption_Key[key_idx][0] == 0 ){
		memcpy( Encryption_Key[key_idx], key[0], 16 );
	}
	else{													// else節の処理は橋本さんの指摘により追加
		memcpy( Encryption_Key[key_idx], key[0], 16 );
	}

	// AES CBCモード
	aes_set_key( Encryption_Key[key_idx], 16, ctx );

	memcpy( indata, outdata, size );
	memset( iv, 0x00, N_BLOCK );	// 初期ベクタ0
	aes_cbc_decrypt( indata, outdata, (size/N_BLOCK), iv, ctx );
}

int DecryptWithKeyAndIV(int KeyAndIVIndex,unsigned char * pIndata,unsigned char *pOutdata, int inSize,int outSize )
{
	int iPadding = 0;
	int bChecked = 0;
	int i = 0;
	uchar ucIV[N_BLOCK];

	if (inSize > outSize || KeyAndIVIndex >= CRYPT_KEY_MAX)
	{
		return 0;
	}

	aes_set_key( key[KeyAndIVIndex], CRYPT_KEY_LENGTH, ctx );
	
	memset( pOutdata, 0x00, outSize );
	memcpy(ucIV,CrpIV[KeyAndIVIndex],sizeof(ucIV));		// 渡すIVは書き換わってしまうので変数へCOPY

	if( aes_cbc_decrypt( pIndata, pOutdata, (inSize/N_BLOCK), ucIV, ctx ) != EXIT_SUCCESS){
		return 0;
	}
	
	// パディングのデータチェック
	iPadding = pOutdata[inSize - 1];
	if (iPadding >= 1 && iPadding <= N_BLOCK && inSize >= iPadding)
	{
		bChecked = 1;
		for (i = 0; i < iPadding; i++)
		{
			if (pOutdata[inSize - 1 - i] != (unsigned char)iPadding)
			{
				bChecked = 0;
				break;
			}
		}
	}

	if (!bChecked)
	{
		return 0;
	}

	return inSize - iPadding;
}
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス (RT精算データ対応)
int DecryptWithKeyAndIV_NoPadding( int KeyAndIVIndex,unsigned char * pIndata, unsigned char *pOutdata, int inSize )
{
	uchar	ucIV[N_BLOCK];

	if( !inSize ){
		return 0;
	}

	// AES CBCモード
	aes_set_key( key[KeyAndIVIndex], CRYPT_KEY_LENGTH, ctx );

	memset( pOutdata, 0x00, inSize );
	memcpy( ucIV,CrpIV[KeyAndIVIndex],sizeof(ucIV) );	// 渡すIVは書き換わってしまうので変数へCOPY

	if( aes_cbc_decrypt( pIndata, pOutdata, (inSize/N_BLOCK), ucIV, ctx ) != EXIT_SUCCESS){
		return 0;
	}else{
		return inSize;
	}
}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス (RT精算データ対応)
#endif

