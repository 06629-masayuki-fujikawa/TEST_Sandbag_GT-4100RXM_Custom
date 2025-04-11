#ifndef AES_SUB_H
#define AES_SUB_H

#include "aes.h"

#if defined( AES_ENC_PREKEYED ) || defined( AES_DEC_PREKEYED )

#define	DATA_SIZE	(N_BLOCK*8)		// 入力データ用バッファサイズ（128byteを超える場合は要変更）

enum{
	CRYPT_KEY_STANDARD = 0,
	CRYPT_KEY_REMOTEDL,			// 遠隔ダウンロード用
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
	CRYPT_KEY_RXMLCDPK,			// 精算機－LCD間通信用
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
// GG129000(S) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
	CRYPT_KEY_PAYEDQR,			// QR駐車券用
// GG129000(E) H.Fujinaga 2022/12/05 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	CRYPT_KEY_QRRECEIPT,		// QR領収証
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	CRYPT_KEY_MAX,
};

#define CRYPT_KEY_LENGTH			16

extern unsigned char iv[], indata[];
extern aes_context ctx[];

#endif

#if defined( AES_ENC_PREKEYED )
void AesCBCEncrypt( unsigned char *outdata, unsigned short size );
// MH810100(S) K.Onodera 2019/11/22 車番チケットレス (RT精算データ対応)
int EncryptWithKeyAndIV_NoPadding( int KeyAndIVIndex,unsigned char * pIndata, unsigned char *pOutdata, int inSize );
// MH810100(E) K.Onodera 2019/11/22 車番チケットレス (RT精算データ対応)
int EncryptWithKeyAndIV(int KeyAndIVIndex,unsigned char * pIndata,unsigned char *pOutdata, int inSize,int outSize );
#endif

#if defined( AES_DEC_PREKEYED )
void AesCBCDecrypt( unsigned char *outdata, unsigned short size );
int DecryptWithKeyAndIV(int KeyAndIVIndex,unsigned char * pIndata,unsigned char *pOutdata, int inSize,int outSize );
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス (RT精算データ対応)
int DecryptWithKeyAndIV_NoPadding( int KeyAndIVIndex,unsigned char * pIndata, unsigned char *pOutdata, int inSize );
// MH810100(E) K.Onodera 2019/11/22 車番チケットレス (RT精算データ対応)
#endif

#endif
