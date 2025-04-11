/*[]----------------------------------------------------------------------[]*/
/*| ec�ް���`                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : G.So                                                     |*/
/*| Date        : 2019-03-15                                               |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
#include	<string.h>
#include	"suica_def.h"
#include	"system.h"

/* ���σ��[�_ �R�}���h */
const unsigned char	ec_tbl[][2] = {
	{ 0xC0, 0x3F },						// �X�^���o�C�v��
	{ 0xC1, 0x3E },						// �ꊇ�v��
	{ 0xC2, 0x3D },						// ���͗v��
	{ 0xC3, 0x3C },						// �o�͎w��
	{ 0xC4, 0x3B },						// ���͗v���̍đ��A�ꊇ�v���̍đ�
	{ 0xC5, 0x3A },						// �o�͎w�߂̍đ�
	{ 0xC2, 0x3D }						// ���͗v����̓��͗v���đ�
};

struct ECCTL_rec	ECCTL;						// ���σ��[�_����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//unsigned char		timer_recept_send_busy;		// 1=���Z����ec������M�^�C�}�[���쒆
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
//EC_BRAND_TBL		RecvBrandTbl[2];			// ��M�����u�����h����e�[�u���\����
EC_BRAND_TBL		RecvBrandTblTmp[2];			// �u�����h�e�[�u����tmp
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
EC_BRAND_TBL		BrandTbl[2];				// �u�����h����e�[�u���\����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//unsigned char		RecvVolumeRes;				// ��M�������ʕύX���ʃf�[�^
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
EC_VOLUME_TBL		VolumeTbl;					// ���ʕύX����e�[�u���\����
EC_MNT_TBL			RecvMntTbl;					// ��M���������e�i���X�e�[�u���\����
EC_MNT_TBL			MntTbl;						// �����e�i���X�e�[�u���\����
EC_BRAND_RES_TBL	RecvBrandResTbl;			// ���[�_��M�����u�����h�I�����ʃe�[�u���\����
EC_BRAND_SEL_TBL	BrandSelTbl;				// �u�����h�I���e�[�u���\����
EC_SETTLEMENT_RES	Ec_Settlement_Res;			// ���ό��ʃf�[�^
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//unsigned char		Ec_Settlement_Sts;			// ���Ϗ�ԃf�[�^
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
t_EC_STATUS_DATA_INFO	EC_STATUS_DATA;			// ��ԃf�[�^
t_EC_STATUS_DATA_INFO	PRE_EC_STATUS_DATA;		// �O���M������ԃf�[�^
t_EC_STATUS_DATA_INFO	EC_STATUS_DATA_WAIT;	// ��ԃf�[�^(��M�҂��p)
unsigned char		Ec_Settlement_Rec_Code[3];	// ���ό��ʏڍ׃G���[�R�[�h
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//unsigned char		ec_Disp_No;					// ���ό��ʏڍ׃G���[�R�[�h�̈ē��\���p�^�[��
//short				ec_MessagePtnNum;			// LCD�\���p�^�[��
//unsigned short		Product_Select_Brand;		// ���i�I���f�[�^���M�̃u�����h�ԍ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG116202(S) JVMA���Z�b�g�񐔐����Ή�
uchar				Ec_Jvma_Reset_Count;		// JVMA���Z�b�g��
// MH810103 GG116202(E) JVMA���Z�b�g�񐔐����Ή�
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
EC_BRAND_RES_RSLT_TBL	BrandResRsltTbl;		// �u�����h�I�����ʉ����e�[�u���\����
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^
// MH810103 GG119202(S) �ُ�f�[�^�t�H�[�}�b�g�ύX
uchar				err_data2;					// �ُ�f�[�^�i2�o�C�g�ځj
// MH810103 GG119202(E) �ُ�f�[�^�t�H�[�}�b�g�ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
EC_SUB_BRAND_TBL	RecvSubBrandTbl;			// �T�u�u�����h�e�[�u���\����
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
