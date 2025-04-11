#ifndef AES_SUB_H
#define AES_SUB_H

#include "aes.h"

#if defined( AES_ENC_PREKEYED ) || defined( AES_DEC_PREKEYED )

#define	DATA_SIZE	(N_BLOCK*8)		// ���̓f�[�^�p�o�b�t�@�T�C�Y�i128byte�𒴂���ꍇ�͗v�ύX�j

enum{
	CRYPT_KEY_STANDARD = 0,
	CRYPT_KEY_REMOTEDL,			// ���u�_�E�����[�h�p
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
	CRYPT_KEY_RXMLCDPK,			// ���Z�@�|LCD�ԒʐM�p
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
	CRYPT_KEY_PAYEDQR,			// QR���Ԍ��p
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	CRYPT_KEY_QRRECEIPT,		// QR�̎���
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

	CRYPT_KEY_MAX,
};

#define CRYPT_KEY_LENGTH			16

extern unsigned char iv[], indata[];
extern aes_context ctx[];

#endif

#if defined( AES_ENC_PREKEYED )
void AesCBCEncrypt( unsigned char *outdata, unsigned short size );
// MH810100(S) K.Onodera 2019/11/22 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
int EncryptWithKeyAndIV_NoPadding( int KeyAndIVIndex,unsigned char * pIndata, unsigned char *pOutdata, int inSize );
// MH810100(E) K.Onodera 2019/11/22 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
int EncryptWithKeyAndIV(int KeyAndIVIndex,unsigned char * pIndata,unsigned char *pOutdata, int inSize,int outSize );
#endif

#if defined( AES_DEC_PREKEYED )
void AesCBCDecrypt( unsigned char *outdata, unsigned short size );
int DecryptWithKeyAndIV(int KeyAndIVIndex,unsigned char * pIndata,unsigned char *pOutdata, int inSize,int outSize );
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
int DecryptWithKeyAndIV_NoPadding( int KeyAndIVIndex,unsigned char * pIndata, unsigned char *pOutdata, int inSize );
// MH810100(E) K.Onodera 2019/11/22 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
#endif

#endif
