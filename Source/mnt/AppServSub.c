/*[]-----------------------------------------------------------------------[]*/
/*| Backup/Restore, Download/Upload common subroutine                       |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005/06/24                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stddef.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"cnm_def.h"
#include	"flp_def.h"
#include	"irq1.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"mif.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"LKcom.h"
#include	"LKmain.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"suica_def.h"
#include	"mnt_def.h"

/*--------------------------------------------------*/
/*	�萔��`										*/
/*--------------------------------------------------*/
#define	ITEM_MAX_1LINE	100						/* CSV�t�@�C�� 1�s�ɃZ�b�g����ő�Item�� */
#define	ITEM_MAX_LOCK_INFO	LOCK_MAX			/* �Ԏ��f�[�^�ő吔 */
#define	SEPARATOR_COMMA		0					/* �Z�p���[�^�Ƃ��ăJ���}���g�p */
#define	SEPARATOR_NEW_LINE	1					/* �Z�p���[�^�Ƃ��ĉ��s�iCRLF�j���g�p */

/*--------------------------------------------------*/
/*	�\���̒�`										*/
/*--------------------------------------------------*/

/*	�Ԏ��f�[�^Upload/Download�p ����f�[�^	*/

typedef struct {			/* BIN��CSV�ϊ������̕ۑ��f�[�^ */
	ushort		CallCount;			/* Call���ꂽ��								*/
} t_AppServ_Bin_toCsv_LockInfo;

typedef struct {			/* CSV��BIN�ϊ������̕ۑ��f�[�^ */
	t_LockInfo	*lock_info_image;	/* �o�C�i���C���[�W�ۑ��G���A�ւ̃|�C���^ 		*/
									/* (���������ɗv��������n�����)				*/
	ulong		image_size;			/* lock_info_image�G���A�̃T�C�Y 				*/
									/* (���������ɗv��������n�����)				*/
	ushort		CallCount;			/* Call���ꂽ��								*/
} t_AppServ_Csv_toBin_LockInfo;

/*	�Ԏ����iFLAPDT�jUpload/Download�p ����f�[�^	*/

typedef struct {					/* CSV��BIN�ϊ������̕ۑ��f�[�^ */
	flp_rec	*	flapdt_image;		/* �o�C�i���C���[�W�ۑ��G���A�ւ̃|�C���^ 		*/
									/* (���������ɗv��������n�����)				*/
	ulong		image_size;			/* lock_info_image�G���A�̃T�C�Y 				*/
									/* (���������ɗv��������n�����)				*/
	ushort		CallCount;			/* Call���ꂽ��								*/
} t_AppServ_Csv_toBin_FLAPDT;

/*--------------------------------------------------*/
/*	�G���A��` (backuped)							*/
/*--------------------------------------------------*/

extern	t_AppServ_LockInfoFukuden	AppServ_LockInfoFukuden;

/*--------------------------------------------------*/
/*	�G���A��`										*/
/*--------------------------------------------------*/
#pragma	section				/* section "B": Initialized data area in external RAM1 */
							/* sys\mendata.c �� "B" section�ɔz�u���Ă��������BOPECTL �Ȃǂ�����ꏊ�ł� */

t_AppServ_BinCsvSave	AppServ_BinCsvSave;		/* �p�����[�^ Upload/Download�p �ۑ��f�[�^ */

t_AppServ_Bin_toCsv_LockInfo	AppServ_B2C_LockInfo;
t_AppServ_Csv_toBin_LockInfo	AppServ_C2B_LockInfo;

ulong AppServ_LogWriteCompleteCheckParam[LOG_STRAGEN];	// LOG Flash�����݊����`�F�b�N�p �p�����[�^ */

// �����Ұ���߰�����ύX
ushort	AppServ_PParam_Copy;					// FLASH �� RAM �����Ұ���߰�L��
												//	OFF:�����Ұ����߰���Ȃ�
												//	ON :�����Ұ����߰����

extern	char	FLT_f_TaskStart;				/* FlashROM�^�X�N�N���t���O */
												/* 0=�N���O�A1=�N����		*/
t_AppServ_Csv_toBin_FLAPDT		AppServ_CtoB_FLAPDT;

/*--------------------------------------------------*/
/*	�e�[�u����`									*/
/*--------------------------------------------------*/
#pragma	section				/* section "C" */

/* ���ʃp�����[�^ �Z�N�V�������A�C�e�����e�[�u�� */
const	ushort	AppServ_CParamSizeTbl[C_PRM_SESCNT_MAX]={
	            1, C_PRM_SES01+1, C_PRM_SES02+1, C_PRM_SES03+1, C_PRM_SES04+1,
	C_PRM_SES05+1, C_PRM_SES06+1, C_PRM_SES07+1, C_PRM_SES08+1, C_PRM_SES09+1,
	C_PRM_SES10+1, C_PRM_SES11+1, C_PRM_SES12+1, C_PRM_SES13+1, C_PRM_SES14+1,
	C_PRM_SES15+1, C_PRM_SES16+1, C_PRM_SES17+1, C_PRM_SES18+1, C_PRM_SES19+1,
	C_PRM_SES20+1, C_PRM_SES21+1, C_PRM_SES22+1, C_PRM_SES23+1, C_PRM_SES24+1,
	C_PRM_SES25+1, C_PRM_SES26+1, C_PRM_SES27+1, C_PRM_SES28+1, C_PRM_SES29+1,
	C_PRM_SES30+1, C_PRM_SES31+1, C_PRM_SES32+1, C_PRM_SES33+1, C_PRM_SES34+1,
	C_PRM_SES35+1, C_PRM_SES36+1, C_PRM_SES37+1, C_PRM_SES38+1, C_PRM_SES39+1
	,		C_PRM_SES40+1, C_PRM_SES41+1
	,	C_PRM_SES42+1, C_PRM_SES43+1, C_PRM_SES44+1, C_PRM_SES45+1, C_PRM_SES46+1
	,	C_PRM_SES47+1, C_PRM_SES48+1, C_PRM_SES49+1, C_PRM_SES50+1, C_PRM_SES51+1
	,	C_PRM_SES52+1, C_PRM_SES53+1, C_PRM_SES54+1, C_PRM_SES55+1, C_PRM_SES56+1
	,	C_PRM_SES57+1, C_PRM_SES58+1, C_PRM_SES59+1, C_PRM_SES60+1 
};

/* �ʃp�����[�^ �Z�N�V�������A�C�e�����e�[�u�� */
static	const	ushort	AppServ_PParamSizeTbl[P_PRM_SESCNT_MAX]={
	1,			P_PRM_SES01+1,	P_PRM_SES02+1,	P_PRM_SES03+1
};

/* �o�b�N�A�b�v�f�[�^�e�[�u�� */
#define	BUP_BACKUP_DATA_COUNT	OPE_DTNUM_MAX			/* �o�b�N�A�b�v�f�[�^�� */

	/** �f�[�^���͕s���̂��ƁB���̏��Ԃ� "OPE_DTNUM_xxxx" �ɏ��� **/
static	const	t_AppServ_AreaInfo	AppServ_BupAreaInfoTbl[BUP_BACKUP_DATA_COUNT]={
	{ (void*)0L				, 0L 					},	// ���ʃp�����[�^ (�Z�b�g���Ȃ�)
	{ (void*)0L				, 0L 					},	// �ʃp�����[�^ (�Z�b�g���Ȃ�)
	{ (void*)&LockInfo[0]	, sizeof(LockInfo) 		},	// �Ԏ��ݒ�
	{ (void*)&LockMaker[0]	, sizeof(LockMaker) 	},	// ���b�N���u�ݒ�
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	{ (void*)&pas_tbl[0]	, sizeof(pas_tbl) 		},	// ����e�[�u��	
	{ (void*)0L				, 0L 					},	// ����e�[�u��	
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	{ (void*)&FLAPDT		, sizeof(FLAPDT) 		},	// ���Ɂi���Ԉʒu�j���
	{ (void*)&Mov_cnt_dat	, sizeof(Mov_cnt_dat) 	},	// ����J�E���g
	{ (void*)&SFV_DAT		, sizeof(SFV_DAT)		},	// NT-NET�f�[�^
	{ (void*)&coin_syu		, sizeof(coin_syu) 		},	// �R�C�����ɏW�v
	{ (void*)&note_syu		, sizeof(note_syu) 		},	// �������ɏW�v
	{ (void*)&turi_kan		, sizeof(turi_kan) 		},	// ���K�Ǘ�
	{ (void*)&sky			, sizeof(sky) 			},	// �W�v�iT,GT,������j
	{ (void*)&skybk			, sizeof(skybk) 		},	// �O��W�v
	{ (void*)&loktl			, sizeof(loktl) 		},	// ���Ԉʒu�ʏW�v
	{ (void*)&loktlbk		, sizeof(loktlbk) 		},	// �O�񒓎Ԉʒu�ʏW�v
	{ (void*)&CarCount		, sizeof(CarCount) 		},	// ���o�ɑ䐔
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	{ (void*)&pas_renewal[0], sizeof(pas_renewal) 	},	// ����X�V�e�[�u��	
	{ (void*)0L				, 0L 					},	// ����X�V�e�[�u��	
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	{ (void*)&cobk_syu		, sizeof(cobk_syu) 		},	// �O��R�C�����ɍ��v
	{ (void*)&nobk_syu		, sizeof(nobk_syu) 		},	// �O�񎆕����ɍ��v
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	{ (void*)&cre_saleng	, sizeof(cre_saleng) 	},	// �N���W�b�g���㋑�ۃf�[�^
//	{ (void*)&cre_uriage	, sizeof(cre_uriage) 	},	// �N���W�b�g����˗��f�[�^
	{ (void*)0L				, 0L					},	// �N���W�b�g���㋑�ۃf�[�^
	{ (void*)0L				, 0L					},	// �N���W�b�g����˗��f�[�^
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	{ (void*)0L				, 0L					},	// Mifare�����ݎ��s�����ް�
	{ (void*)&SUICA_LOG_REC_FOR_ERR		, sizeof(SUICA_LOG_REC_FOR_ERR) },	// ���ψُ킪�����������̃��O���
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
//	{ (void*)&Syuukei_sp	, sizeof(Syuukei_sp)	},	// Edy,Suica�W�v���
	{ (void*)0L				, 0L					},	// Edy,Suica�W�v���
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
	{ (void*)&Attend_Invalid_table	, sizeof(Attend_Invalid_table)},	// �W�������ް�
	{ (void*)0L				, 0L 					},	// ���o�Ƀ��O
	{ (void*)bk_LockInfo	, sizeof(bk_LockInfo) 	},	// �Ԏ��ݒ�o�b�N�A�b�v�G���A�i�Ԏ��̏�p�j
	{ (void*)&SetDiffLogBuff, sizeof(SetDiffLogBuff)},	// �ݒ�X�V����
	{ (void*)&LongTermParkingPrevTime, sizeof(LongTermParkingPrevTime)},	// �������ԃf�[�^�O�񔭕񎞊�
	{ (void*)&Rau_SeqNo		, sizeof(Rau_SeqNo)		},	// �Z���^�[�ǔ�
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	{ (void*)&cre_slipno, sizeof(cre_slipno)},			// �N���W�b�g�[�������ʔ�
	{ (void*)0L				, 0L					},	// �N���W�b�g�[�������ʔ�
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	{ (void*)&Date_Syoukei, sizeof(Date_Syoukei)},		//���t�ؑ֊�̏��v(���A���^�C���f�[�^�p)
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
	{ (void*)&EcEdyTerminalNo[0], sizeof(EcEdyTerminalNo)},	// Edy��ʒ[��ID
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
//	{ (void*)&EC_ALARM_LOG_DAT, sizeof(EC_ALARM_LOG_DAT)},	// ���σ��[�_ �A���[��������O
	{ (void*)0L				, 0L					},	// ���σ��[�_ �A���[��������O
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
	{ (void*)&DC_SeqNo		, sizeof(DC_SeqNo)		}, // DC-NET�ʐM�p�Z���^�[�ǔ�
	{ (void*)&REAL_SeqNo	, sizeof(REAL_SeqNo)	}, // ���A���^�C���ʐM�p�Z���^�[�ǔ�
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	{ (void*)&EC_ALARM_LOG_DAT, sizeof(EC_ALARM_LOG_DAT)},	// ���σ��[�_ �A���[��������O
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
	{ (void*)&RecvBrandTbl[0], sizeof(RecvBrandTbl)	},	// ���σ��[�_�����M�����u�����h�e�[�u��
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	{ (void*)&Syuukei_sp	, sizeof(Syuukei_sp)	},	// SX,���σ��[�_�W�v���
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
};

/* �Ώۃf�[�^FROM�ۑ��̈��� */
/** �f�[�^���͕s���̂��ƁB���̏��Ԃ� "OPE_DTNUM_xxxx" �ɏ��� **/
static	const	t_AppServ_FromInfo	AppServ_BupAreaFromInfoTbl[BUP_BACKUP_DATA_COUNT]={
// NOTE:�]���̃o�b�N�A�b�v�͎w�肵��FROM�̃G���A�ɑΏۂ�RAM�̈����C�ɃR�s�[��������ł��邪
// FT-4000�̏ꍇ�A�V���A��FROM��256Byte�P�ʂ̏����������ł��Ȃ��d�l�̂���RAM�̈悲�ƂɃZ�N�^(�A�h���X)��ݒ肵�Ă���
// �A�h���X
	0                  ,	// ���ʃp�����[�^ (�Z�b�g���Ȃ�)
	0                  ,	// �ʃp�����[�^ (�Z�b�g���Ȃ�)
	FLT_BACKUP_SECTOR1 ,	// �Ԏ��ݒ�
	FLT_BACKUP_SECTOR2 ,	// ���b�N���u�ݒ�
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	FLT_BACKUP_SECTOR3 ,	// ����e�[�u��	
	0                  ,	// ����e�[�u��	
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	FLT_BACKUP_SECTOR4 ,	// ���Ɂi���Ԉʒu�j���
	FLT_BACKUP_SECTOR5 ,	// ����J�E���g
	FLT_BACKUP_SECTOR6 ,	// NT-NET�f�[�^
	FLT_BACKUP_SECTOR7 ,	// �R�C�����ɏW�v
	FLT_BACKUP_SECTOR8 ,	// �������ɏW�v
	FLT_BACKUP_SECTOR9 ,	// ���K�Ǘ�
	FLT_BACKUP_SECTOR10,	// �W�v�iT,GT,������j
	FLT_BACKUP_SECTOR11,	// �O��W�v
	FLT_BACKUP_SECTOR12,	// ���Ԉʒu�ʏW�v
	FLT_BACKUP_SECTOR13,	// �O�񒓎Ԉʒu�ʏW�v
	FLT_BACKUP_SECTOR14,	// ���o�ɑ䐔
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	FLT_BACKUP_SECTOR15,	// ����X�V�e�[�u��	
	0                  ,	// ����X�V�e�[�u��	
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
	FLT_BACKUP_SECTOR16,	// �O��R�C�����ɍ��v
	FLT_BACKUP_SECTOR17,	// �O�񎆕����ɍ��v
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	FLT_BACKUP_SECTOR18,	// �N���W�b�g�����񃍃O
//	FLT_BACKUP_SECTOR19,	// �N���W�b�g������W�`�F�b�N�G���A
	0                  ,	// �N���W�b�g�����񃍃O
	0                  ,	// �N���W�b�g������W�`�F�b�N�G���A
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	0                  ,	// Mifare�����ݎ��s�����ް�
	FLT_BACKUP_SECTOR28,	// ���ψُ킪�����������̃��O���
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	FLT_BACKUP_SECTOR20,	// Edy,Suica�W�v���
	0                  ,	// Edy,Suica�W�v���
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
	FLT_BACKUP_SECTOR21,	// �W�������ް�
	0                  ,	// ���o�Ƀ��O
	FLT_BACKUP_SECTOR23,	// �Ԏ��ݒ�o�b�N�A�b�v�G���A�i�Ԏ��̏�p�j
	FLT_BACKUP_SECTOR24,	// �ݒ�X�V����
	FLT_BACKUP_SECTOR25,	// �������ԃf�[�^�O�񔭕񎞊�
	FLT_BACKUP_SECTOR29,	// �Z���^�[�ǔ�
// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
//	FLT_BACKUP_SECTOR30,	// �N���W�b�g�[�������ʔ�
	0                  ,	// �N���W�b�g�[�������ʔ�
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	FLT_BACKUP_SECTOR31,	//���t�ؑ֊�̏��v(���A���^�C���f�[�^�p)
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
	FLT_BACKUP_SECTOR32,	// Edy��ʒ[��ID
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
//	FLT_BACKUP_SECTOR33,	// ���σ��[�_ �A���[��������O
	0					,	// ���σ��[�_ �A���[��������O
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
	FLT_BACKUP_SECTOR34,	// DC-NET�ʐM�p�Z���^�[�ǔ�
	FLT_BACKUP_SECTOR35,	// ���A���^�C���ʐM�p�Z���^�[�ǔ�
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	FLT_BACKUP_SECTOR36,	// ���σ��[�_ �A���[��������O
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
	FLT_BACKUP_SECTOR37,	// ���σ��[�_�����M�����u�����h�e�[�u��
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	FLT_BACKUP_SECTOR38,	// SX,���σ��[�_�W�v���
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
};

/* SUM�Z�o�Ώۃf�[�^��� */
/** �f�[�^���͕s���̂��ƁB���̏��Ԃ� "OPE_DTNUM_xxxx" �ɏ��� **/
static	const	t_AppServ_AreaInfo	AppServ_SumAreaInfoTbl[BUP_BACKUP_DATA_COUNT]={
	{ (void*)&CParam.CParam01[0]	, sizeof(CParam)-sizeof(CParam.CParam00)	},	// ���ʃp�����[�^
	{ (void*)0L				, 0L 					},	// �ʃp�����[�^
	{ (void*)&LockInfo[0]	, sizeof(LockInfo) 		},	// �Ԏ��ݒ�
	{ (void*)0L				, 0L 					},	// ���b�N���u�ݒ�
	{ (void*)0L				, 0L 					},	// ����e�[�u��	
	{ (void*)0L				, 0L 					},	// ���Ɂi���Ԉʒu�j���
	{ (void*)0L				, 0L 					},	// ����J�E���g
	{ (void*)0L				, 0L 					},	// NT-NET�f�[�^
	{ (void*)0L				, 0L 					},	// �R�C�����ɏW�v
	{ (void*)0L				, 0L 					},	// �������ɏW�v
	{ (void*)0L				, 0L 					},	// ���K�Ǘ�
	{ (void*)0L				, 0L 					},	// �W�v�iT,GT,������j
	{ (void*)0L				, 0L 					},	// �O��W�v
	{ (void*)0L				, 0L 					},	// ���Ԉʒu�ʏW�v
	{ (void*)0L				, 0L 					},	// �O�񒓎Ԉʒu�ʏW�v
	{ (void*)0L				, 0L					},	// ���o�ɑ䐔
	{ (void*)0L				, 0L					},	// ����X�V�e�[�u��
	{ (void*)0L				, 0L					},	// �O��R�C�����ɍ��v
	{ (void*)0L				, 0L					},	// �O�񎆕����ɍ��v
	{ (void*)0L				, 0L					},	// �N���W�b�g�����񃍃O
	{ (void*)0L				, 0L					},	// �N���W�b�g������W�`�F�b�N�G���A
	{ (void*)0L				, 0L					},	// Mifare�����ݎ��s�����ް�
	{ (void*)0L				, 0L					},	// Suica�ُ탍�O
	{ (void*)0L				, 0L					},	// Edy,Suica�W�v���
	{ (void*)0L				, 0L					},	// �W�������ް�
	{ (void*)0L				, 0L					},	// ���o�Ƀ��O
	{ (void*)0L				, 0L					},	// �Ԏ��ݒ�o�b�N�A�b�v�G���A�i�Ԏ��̏�p�j
	{ (void*)0L				, 0L					},	// �ݒ�X�V����
	{ (void*)0L				, 0L					},	// �������ԃf�[�^�O�񔭕񎞊�
	{ (void*)0L				, 0L					},	// �Z���^�[�ǔ�
	{ (void*)0L				, 0L					},	// �N���W�b�g�[�������ʔ�
// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
	{ (void*)0L				, 0L					},	//���t�ؑ֊�̏��v(���A���^�C���f�[�^�p)
// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
// MH321800(S) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
	{ (void*)0L				, 0L					},	// Edy��ʒ[��ID
	{ (void*)0L				, 0L					},	// ���σ��[�_ �A���[��������O
// MH321800(E) hosoda IC�N���W�b�g�Ή� (�W�v���O/Edy�̏�ʒ[��ID/�A���[��������O)
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
	{ (void*)0L				, 0L					},	// DC-NET�ʐM�p�Z���^�[�ǔ�
	{ (void*)0L				, 0L					},	// ���A���^�C���ʐM�p�Z���^�[�ǔ�
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�o�b�N�A�b�v)
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	{ (void*)0L				, 0L					},	// ���σ��[�_ �A���[��������O
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
	{ (void*)0L				, 0L					},	// ���σ��[�_�����M�����u�����h�e�[�u��
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
	{ (void*)0L				, 0L					},	// SX,���σ��[�_�W�v���
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
};

/*--------------------------------------------------*/
/*	�����֐��v���g�^�C�v��`						*/
/*--------------------------------------------------*/
#pragma	section				/* section "P" */

void	AppServ_CnvParam2CSV_Sub1( char *csvdata, ushort *csvdata_len );
void	AppServ_CnvParam2CSV_Sub2( char *csvdata, ushort *csvdata_len,
								   uchar ParamKind, ushort SectionNum, ushort ItemNum );
uchar	AppServ_CnvParam2CSV_Sub3( char *csvdata, ushort *csvdata_len );

ulong	AppServ_ConvCsvParam_ImageAreaSizeGet( const t_AppServ_ParamInfoTbl *param_info );
uchar	AppServ_CnvCsvParam_1ColumnRead( const char *CsvData, ulong *pBinData, const char **pNextCsvData );
uchar	AppServ_CnvCsvParam_SecItemNumGet( const char *CsvData, uchar *pCParam, ushort *pSecNum, ushort *pItemNum );
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
//ushort	AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( char SecKind, ushort SecNum );
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)

uchar	AppServ_CnvLockInfo2CSV_SUB(ulong param, char *buf, uchar separator);
#define	_appserv_CheckVal(data, type, member)	appserv_CheckVal(data, sizeof(((type*)0)->member))

int		appserv_CheckVal(ulong data, size_t size);

void	AppServ_ConvErrArmCSV_Sub1		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvErrArmCSV_Sub2		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
uchar	AppServ_ConvErrArmCSV_Sub3		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvErrDataCreate		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvArmDataCreate		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvMoniDataCreate		( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	AppServ_ConvOpeMoniDataCreate	( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
void	OccurReleaseDataEdit			( char *, ulong *, uchar , t_AppServ_Bin_toCsv_Param * );
void	TimeDataEdit					( char *, ulong *, date_time_rec * );
void	LevelDataEdit					( char *, ulong *, uchar );
uchar	DigitNumCheck					( long );
ulong	DataSizeCheck					( char*, uchar );
void	AppServ_ConvCarInfoDataCreate	( char *, ulong *, t_AppServ_Bin_toCsv_Param * );
uchar	AppServ_CnvCsvFLAPDT_1ColumnRead( const char *CsvData, long *pBinData, const char **pNextCsvData );
void	AppServ_ConvChk_ResultCSV_Sub1( char *csvdata, ulong *csvdata_len );
void	AppServ_ConvChk_ResultCSV_Sub2( char *csvdata, ulong *csvdata_len,ushort SectionNum, ushort ItemNum );
uchar	AppServ_ConvChk_ResultCSV_Sub3( char *csvdata, ulong *csvdata_len );


/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁE�ʃp�����[�^ ���e�[�u���쐬									|*/
/*|																			|*/
/*|	�p�����[�^�f�[�^�o�b�N�A�b�v�p��񂻂��Ďg�p����f�[�^�̍쐬�B			|*/
/*|	�p�����[�^�̃Z�N�V�������ƃZ�N�V�������̃A�C�e�������Z�b�g����B		|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : �f�[�^�i�[�G���A�ւ̃|�C���^								|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*|	���u�N���� �{�֐�Call�O�� prm_init() �� Call����Ă��邱��				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_MakeParamInfoTable( t_AppServ_ParamInfoTbl *pTable )
{
	ushort	i;

	/* �L���Z�N�V������ set */
	pTable->csection_num = C_PRM_SESCNT_MAX;
	pTable->psection_num = P_PRM_SESCNT_MAX;

	/** section data top address & item count in section set **/

	/* ���ʃp�����[�^ */
	for( i=0; i< C_PRM_SESCNT_MAX; ++i ){
		pTable->csection[i].address  = CPrmSS[i];
		pTable->csection[i].item_num = AppServ_CParamSizeTbl[i];
	}

	/* �ʃp�����[�^ */
	for( i=0; i< P_PRM_SESCNT_MAX; ++i ){
		pTable->psection[i].address  = PPrmSS[i];
		pTable->psection[i].item_num = AppServ_PParamSizeTbl[i];
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	�o�b�N�A�b�v�f�[�^���e�[�u���쐬										|*/
/*|																			|*/
/*|	���̑��f�[�^�o�b�N�A�b�v�p��񂻂��Ďg�p����f�[�^�̍쐬�B				|*/
/*|	�f�[�^���̐擪�A�h���X�ƃ����O�X���Z�b�g����B							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : �f�[�^�i�[�G���A�ւ̃|�C���^								|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_MakeBackupDataInfoTable( t_AppServ_AreaInfoTbl *pTable )
{
	ushort	i;
	pTable->area_num = BUP_BACKUP_DATA_COUNT;				/* area count set */

	for( i=0; i<BUP_BACKUP_DATA_COUNT; ++i ){
		pTable->area[i].address = AppServ_BupAreaInfoTbl[i].address;
		pTable->area[i].size    = AppServ_BupAreaInfoTbl[i].size;
		pTable->from_area[i].from_address = AppServ_BupAreaFromInfoTbl[i].from_address;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	�w�b�_�^�t�b�^���M�f�[�^�t�H�[�}�b�g�쐬								|*/
/*|																			|*/
/*|	PC����_�E�����[�h���ꂽ�f�[�^��FlashROM�i�[�`���ɕϊ�����B			|*/
/*|	�����f�[�^��ASCII or Shift-JIS�ɑΉ��BUnicode�͕s�B					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : formated = �ϊ���f�[�^�i�[�G���A�ւ̃|�C���^			|*/
/*|				   text = �ϊ����f�[�^(PC�����M�����f�[�^)�ւ̃|�C���^	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_FormatHeaderFooter( char formated[4][36], const char *text, ushort text_len )
{
	ushort	Line;
	ushort	Col;
	ushort	i;

	memset( &formated[0][0], ' ', 4*36 );			/* set area space clear */

	Line = 0;
	Col  = 0;
	for( i=0; i<text_len; ++i ){
		if( 0x0d == text[i] ){
			++Line;
			/* �s��Max�`�F�b�N */
			if( 4 <= Line ){
				return;
			}
			Col = 0;
			if( 0x0a == text[i+1] ){
				++i;
			}
			continue;
		}
		else{
			/* ������Max�`�F�b�N */
			if( 36 <= Col ){
				continue;
			}
			formated[Line][Col++] = text[i];
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	�w�b�_�^�t�b�^���M�f�[�^�t�H�[�}�b�g�쐬								|*/
/*|																			|*/
/*|	���Z�@���f�[�^�ɉ��s�R�[�h(CRLF)��}������								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : inserted = ���s�}����f�[�^�i�[�G���A�ւ̃|�C���^		|*/
/*|				   text = ���s�}�����f�[�^(���Z�@���f�[�^)�ւ̃|�C���^		|*/
/*| RETURN VALUE : �ϊ���̃f�[�^��										    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : machida													|*/
/*| Date         : 2005/06/30												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	AppServ_InsertCrlfToHeaderFooter( char *inserted, const char text[4][36] )
{
	ushort	i, len;
	
	len = 0;
	for (i = 0; i < 4; i++) {
		/* copy 1 line */
		memcpy(&inserted[len], &text[i][0], 36);
		len += 36;
		/* insert cr & lf */
		inserted[len++] = '\r';
		inserted[len++] = '\n';
	}
	
	return len;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��i�������j				|*/
/*|																			|*/
/*|	RAM���f�[�^��CSV�`���l�ɕϊ�����B���̂��߂̑O�����B					|*/
/*|	findfirst, findnext�̂悤�ɍŏ��ɖ{�֐��A�ȍ~��AppServ_CnvParam2CSV()��	|*/
/*|	�����Ǐo�����s���B														|*/
/*|	���� datasize�ɂ�1112byte�ȏ��p�ӂ��邱�ƁB							|*/
/*|		 AppServ_CnvParam2CSV()��CSV��s���̃f�[�^���i100�A�C�e�����j��		|*/
/*|		 �Z�b�g���邽�߁B													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = ���ʁE�ʃp�����[�^�f�[�^�̏��e�[�u��	|*/
/*|								�ւ̃|�C���^								|*/
/*|				   datasize = AppServ_CnvParam2CSV()�ň��ɃZ�b�g�\��	|*/
/*|							  �f�[�^�T�C�Y�B�Ăь����W���[���̃o�b�t�@�T�C�Y|*/
/*| RETURN VALUE : 1 = OK                                                   |*/
/*|				   0 = NG (���[�N�G���A�T�C�Y��������)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_ConvParam2CSV_Init( const t_AppServ_ParamInfoTbl *param_info, ulong datasize )
{
	#define	WORK_BUF_MIN_LENGTH		1112

	if( WORK_BUF_MIN_LENGTH > datasize ){		/* work area size error (Y) */
		return	(uchar)0;
	}

	/* ��M�f�[�^�ۑ� */
	memcpy( &AppServ_BinCsvSave.param_info, param_info, sizeof(t_AppServ_ParamInfoTbl) );
	AppServ_BinCsvSave.Bin_toCsv_Param.datasize = datasize;

	/* ��ƃG���A�N���A */
	AppServ_BinCsvSave.Bin_toCsv_Param.Phase	 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.Kind		 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.Section	 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.Item		 = 0;
	AppServ_BinCsvSave.Bin_toCsv_Param.CallCount = 0;

	return	(uchar)1;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��i�Ǐo���j				|*/
/*|																			|*/
/*|	RAM���f�[�^��CSV�`���l�ɕϊ����A�o�b�t�@�ɃZ�b�g����B					|*/
/*|	�f�[�^��100�� ���� 1�Z�N�V�������̏��������ƂȂ�B						|*/
/*|	�{�֐�Call�O�� AppServ_ConvParam2CSV_Init() ��Call���邱�ƁB			|*/
/*|	�ŏ�����ǂݏo���Ȃ����ꍇ���A�������֐���Call���邱�ƁB				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*|				   finish = �S�Z�N�V�����̕ϊ������� 1���Z�b�g�B�ȊO��0�B	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_CnvParam2CSV( char *csvdata, ushort *csvdata_len, uchar *finish )
{
	ushort	TotalLen;
	uchar	ret;

	++AppServ_BinCsvSave.Bin_toCsv_Param.CallCount;			/* Call���ꂽ�� +1 		*/
	*finish = 0;

	if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Phase ){	/* �擪�s�i���o���j���M�t�F�[�Y (Y)	*/
		AppServ_CnvParam2CSV_Sub1( csvdata, csvdata_len );	/* ���o���쐬 */
		AppServ_BinCsvSave.Bin_toCsv_Param.Phase = 1;		/* ���t�F�[�Y�̓f�[�^���ϊ� */
		return;
	}

	/** �f�[�^���ϊ��t�F�[�Y **/

	TotalLen = 0;

	AppServ_CnvParam2CSV_Sub2( csvdata, &TotalLen, 			/* �擪�J�������o���쐬 */
									AppServ_BinCsvSave.Bin_toCsv_Param.Kind,
									AppServ_BinCsvSave.Bin_toCsv_Param.Section,
									(ushort)AppServ_BinCsvSave.Bin_toCsv_Param.Item );

	/* 1�Z�N�V�����Z�b�g���� ���� �����݃o�b�t�@�T�C�YLimit�܂Ńf�[�^�Z�b�g */
	ret = AppServ_CnvParam2CSV_Sub3( csvdata, &TotalLen );
	*csvdata_len = TotalLen;
	if( ret == 1 ){											/* �I�� (Y) */
		*finish = 1;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub1�j					|*/
/*|																			|*/
/*|	CSV�t�@�C���̂P�s�ڌ��o�����쐬����B									|*/
/*|	�T�C�Y��493byte�ƂȂ邪�A�������ݗp�o�b�t�@��512byte�ȏ�ł���̂�		|*/
/*|	��C�Ɉ�s�����Z�b�g����B												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_CnvParam2CSV_Sub1( char *csvdata, ushort *csvdata_len )
{
	ushort	i;
	ushort	SetLen;
	ushort	TotalLen;
	uchar	Asc[8];

	TotalLen = 0;
	Asc[0] = ',';
	Asc[1] = '[';

	for( i=0; i<100; ++i ){

		cnvdec4( (char*)&Asc[2], (long)i );						/* Bin to Ascii�ϊ� */

		SetLen = 0;
		if( 10 > i ){										/* �����P���� (Y) */
			Asc[2] = Asc[5];
			Asc[3] = ']';
			SetLen = 4;
		}
		else{												/* �����Q���� */
			Asc[2] = Asc[4];
			Asc[3] = Asc[5];
			Asc[4] = ']';
			SetLen = 5;
		}

		memcpy( (void*)&csvdata[TotalLen], (const void*)&Asc[0], (size_t)SetLen );
		TotalLen += SetLen;
	}

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;

	*csvdata_len = TotalLen + 2;
	return;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub2�j					|*/
/*|																			|*/
/*|	�e�s�̐擪�i�P�J�����ځj���o���쐬����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*|				   ParamKind = 0:���ʁC 1:��								|*/
/*|				   SectionNum = �Z�N�V�����ԍ��i1�`99�j						|*/
/*|				   ItemNum = �擪��Item�ԍ��i1�`9999�j						|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_CnvParam2CSV_Sub2( char *csvdata, ushort *csvdata_len,
								   uchar ParamKind, ushort SectionNum, ushort ItemNum )
{
	csvdata[0] = '[';

	if( 0 == ParamKind )
		csvdata[1] = 'C';
	else
		csvdata[1] = 'P';

	cnvdec2( &csvdata[2], (short)SectionNum );
	csvdata[4] = '-';
	cnvdec4( &csvdata[5], (long)ItemNum );
	csvdata[9] = ']';
	*csvdata_len = 10;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub3�j					|*/
/*|																			|*/
/*|	�f�[�^�Z�b�g�iCSV�쐬�j����												|*/
/*| 1�s���̃f�[�^���Z�b�g�B�i�P�Z�N�V�����I�� ���� 100�A�C�e���j			|*/
/*|	�����ɂ�CR,LF���Z�b�g����B(Max1112byte�Z�b�g����)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*|				   ��ƃG���A�iAppServ_BinCsvSave�j�̏����g���B			|*/
/*				   �X�V���s���B												|*/
/*| RETURN VALUE : 1=1�Z�N�V�����I���B 0=�����݃o�b�t�@Limit�ɂ��I��      |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvParam2CSV_Sub3( char *csvdata, ushort *csvdata_len )
{
	ushort	TgtSection;				/* target Section number (0- ) */
	ushort	TgtTotalItemCount;		/* total Item count in target Section */

	ushort	TgtItemNum;				/* ���ɍ쐬���� Item�ԍ� (0- ) */
	ulong	*pTgtSectionTop;		/* Top item data address in target Section */
	ushort	TgtSectionCount;		/* Section count in target Section */

	ushort	TotalLen;				/* �������񂾃T�C�Y */
	ushort	SetItemCount;			/* ���񏑂�����Item�� */
	uchar	Asc[12];
	uchar	ThisItemLen;

	/* �p�����[�^���e�[�u������ target���̏�񒊏o */
	TgtSection	= AppServ_BinCsvSave.Bin_toCsv_Param.Section;
	if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Kind ){		/* ���ʃp�����[�^ (Y) */
		TgtTotalItemCount = AppServ_BinCsvSave.param_info.csection[TgtSection].item_num;
		pTgtSectionTop    = AppServ_BinCsvSave.param_info.csection[TgtSection].address;
		TgtSectionCount   = AppServ_BinCsvSave.param_info.csection_num;
	}
	else{													/* �ʃp�����[�^ */
		TgtTotalItemCount = AppServ_BinCsvSave.param_info.psection[TgtSection].item_num;
		pTgtSectionTop    = AppServ_BinCsvSave.param_info.psection[TgtSection].address;
		TgtSectionCount   = AppServ_BinCsvSave.param_info.psection_num;
	}

	TotalLen = *csvdata_len;
	SetItemCount = 0;										/* ���񏑂�����Item���N���A */
	TgtItemNum = AppServ_BinCsvSave.Bin_toCsv_Param.Item;	/* target Item number */

	/* 1�Z�N�V�����Z�b�g���� ���� �����݃o�b�t�@�T�C�YLimit�܂� */
	while( (TgtItemNum < TgtTotalItemCount) && (SetItemCount < ITEM_MAX_1LINE) ){
		Asc[0] = ',';

		ThisItemLen = intoasl_0sup( &Asc[1], pTgtSectionTop[ TgtItemNum ], (unsigned short)10 );
							/* Long size Hex data change to Decimal Ascii (0 suppress) */
		memcpy( (void*)&csvdata[TotalLen], (const void*)&Asc[0], (size_t)(1+ThisItemLen) );
		TotalLen += (1+ThisItemLen);

		++SetItemCount;
		++TgtItemNum;
	}

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;
	TotalLen += 2;

	*csvdata_len = TotalLen;

	/* �I���v������ */
	if( TgtItemNum >= TgtTotalItemCount ){		/* 1�Z�N�V�����Z�b�g���� */
		++AppServ_BinCsvSave.Bin_toCsv_Param.Section;			/* ���̃Z�N�V������ */
		AppServ_BinCsvSave.Bin_toCsv_Param.Item	= 0;

		if( TgtSectionCount <= AppServ_BinCsvSave.Bin_toCsv_Param.Section ){
																/* �S�Z�N�V�����I�� (Y) */

			if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Kind ){	/* ���ʃp�����[�^ (Y) */
				return	1;		/* �S�I�� */
			}
		}
	}

	else{	/* �r���ŏI�������ꍇ�́AIndex���Z�[�u���� */
		AppServ_BinCsvSave.Bin_toCsv_Param.Item	= TgtItemNum;
	}
	return	0;
}


/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�_�E�����[�h���̃f�[�^�ϊ��i�������j				|*/
/*|																			|*/
/*|	PC�����M����CSV�`���f�[�^��RAM���ۑ��`���ɕϊ�����B���̂��߂̑O�����B|*/
/*|	findfirst, findnext�̂悤�ɍŏ��ɖ{�֐��A�ȍ~��AppServ_CnvCSV2Param()��	|*/
/*|	�����Ǐo�����s���B														|*/
/*|	���� image_size�͋��ʁ��ʃp�����[�^�T�C�Y�i+4byte�j�ȏ���Z�b�g���邱�ƁB	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = ���ʁE�ʃp�����[�^�f�[�^�̏��e�[�u��	|*/
/*|								�ւ̃|�C���^								|*/
/*|				   param_image = �ϊ����RAM�C���[�W�f�[�^�i�[�G���A�ւ�	|*/
/*|								 �|�C���^									|*/
/*|								�C���[�W�f�[�^�͋���,�ʂ̏��Ƀf�[�^������	|*/
/*|								�l�ɃZ�b�g����B							|*/
/*|								�iCParam,PParam�C���[�W��A�����ăZ�b�g�j	|*/
/*|				   datasize = param_image�̃T�C�Y							|*/
/*| RETURN VALUE : 1 = OK                                                   |*/
/*|				   0 = NG (�C���[�W�G���A�T�C�Y��������)					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_ConvCSV2Param_Init( const t_AppServ_ParamInfoTbl *param_info, 
									char *param_image, ulong image_size )
{
	ulong	wkl;

	wkl = AppServ_ConvCsvParam_ImageAreaSizeGet( param_info );	/* RAM�C���[�W�G���A�T�C�Yget */
	if( image_size < wkl ){										/* ��ƃG���A�������� (Y) */
		return	(uchar)0;
	}

	/* ��M�f�[�^�ۑ� */
	memcpy( &AppServ_BinCsvSave.param_info, (const void*)param_info, sizeof(t_AppServ_ParamInfoTbl) );
	AppServ_BinCsvSave.Csv_toBin_Param.param_image = param_image;
	AppServ_BinCsvSave.Csv_toBin_Param.image_size = image_size;

	/* ��ƃG���A�N���A */
	memset( param_image, 0, image_size );
	memcpy(param_image, &CParam, sizeof(CParam));
	AppServ_BinCsvSave.Bin_toCsv_Param.CallCount = 0;

	return	(uchar)1;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^ �_�E�����[�h�T�u���[�`��							|*/
/*|	RAM���C���[�W�t�@�C���̃T�C�Y�擾										|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = ���ʁE�ʃp�����[�^�f�[�^�̏��e�[�u��	|*/
/*|								�ւ̃|�C���^								|*/
/*| RETURN VALUE : RAM��C���[�W�t�@�C���̃T�C�Y��Ԃ�                      |*/
/*|				   CParam00�`PParamEnd�܂ł̃T�C�Y							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	AppServ_ConvCsvParam_ImageAreaSizeGet( const t_AppServ_ParamInfoTbl *param_info )
{
	return (ulong)(sizeof(CParam) + sizeof(PParam)) ;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�_�E�����[�h���̃f�[�^�ϊ��i�Ǐo���j				|*/
/*|																			|*/
/*|	PC�����M����CSV�`���f�[�^��RAM���ۑ��`���ɕϊ�����B					|*/
/*|	�{�֐�Call�O�� AppServ_ConvCSV2Param_Init() ��Call���邱�ƁB			|*/
/*|	�ŏ�����ǂݏo���Ȃ����ꍇ���A�������֐���Call���邱�ƁB				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PC�����M����CSV�`���f�[�^�i�P�s���j			|*/
/*|							 �ւ̃|�C���^									|*/
/*|				   csvdata_len = csvdata�̃T�C�Y							|*/
/*|				   finish = �S�Z�N�V�����̕ϊ������� �Z�b�g�����������O�X��	|*/
/*|							�Z�b�g�B�ȊO��0�B								|*/
/*|				   �������� CR ���� EOF���Z�b�g���Ă��������B				|*/
/*| RETURN VALUE : 1=OK, 0=CSV�`��NG                                        |*/
/*|					   �Z�N�V�����ԍ��C�A�C�e�������͈͊O(�I�[�o�[)�`�F�b�N	|*/
/*|					   �́A�擪�J�����̃R�����g�݂̂��`�F�b�N�ΏۂƂ��A		|*/
/*|					   �ȍ~�A�C�e�������I�[�o�[�����ꍇ�́A�f�[�^�͏�������	|*/
/*|					   �Ȃ������Ő���I���Ƃ���B							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvCSV2Param(const char *csvdata, ushort csvdata_len, ulong *finish )
{
	/** return code **/
	#define	BIT_DATA_EXIST	1								/* �f�[�^����bit */
	#define	BIT_NEXT_EXIST	2								/* ���f�[�^���� */
	#define	BIT_COMMENT		4								/* �R�����g�J���� */
	#define	BIT_ERROR		0x80							/* �G���[���� */

	ushort	i;
	uchar	ret;
	ulong	BinData;
	const char	*pNextCsvData;
	ushort	SecNum, ItemNum;
	ushort	TgtSecTopIndex;			/* ���[�N�G���A�擪����̑�Index�� */
	ulong	*pTgtSecTop;			/* �ړI�Z�N�^�[�̐擪�A�C�e���A�h���X */
	char	f_CParam;				/* 1=���ʃp�����[�^���Ώ� */
	ushort	MaxSecNum;				/* �Z�N�V������ */
	ushort	MaxItemNum;				/* �Y���Z�N�V�����̃A�C�e���� */

	++AppServ_BinCsvSave.Bin_toCsv_Param.CallCount;			/* Call���ꂽ�� +1 		*/
	*finish = 0;

	/* 1�J�����ϊ� */
	ret = AppServ_CnvCsvParam_1ColumnRead( csvdata, &BinData, &pNextCsvData );

	if( (0 == (ret & BIT_DATA_EXIST)) && (0 == (ret & BIT_COMMENT)) ){
															/* �f�[�^�Ȃ�(�����Ȃ�J���}:Y) */
		/* �擪�s�Ɣ��f */
		return	(uchar)1;									/* �I�� */
	}

	if( 0 == (ret & BIT_COMMENT) ){							/* �R�����g�J����(N) */
		*finish = 1;
		return	(uchar)0;									/* �ُ�I�� */
	}														/* �擪���R�����g�łȂ��̂͌`���ُ� */

	if( 0 == (ret & BIT_NEXT_EXIST) ){						/* ���f�[�^�Ȃ�(Y) */
		return	(uchar)1;									/* �I�� */
	}

	/* �擪�J�������� �^�[�Q�b�g�̃Z�N�V�����ԍ��Ɛ擪�A�C�e���ԍ��𓾂� */
	if( BIT_ERROR == AppServ_CnvCsvParam_SecItemNumGet((const char*)csvdata, (uchar*)&f_CParam, &SecNum, &ItemNum) ){
		*finish = 1;
		return	(uchar)0;									/* �ُ�I�� */
	}														/* �R�����g�`���ُ� */

	/* �Z�N�V���� �y�� �A�C�e���̍ő吔�擾 */
	if( 1 == f_CParam ){									/* ���ʃp�����[�^ (Y) */
		MaxSecNum = AppServ_BinCsvSave.param_info.csection_num;	/* Max�Z�N�V������ get */
		MaxItemNum = AppServ_BinCsvSave.param_info.csection[SecNum].item_num;
															/* Max�A�C�e���� get */
	}
	else{													/* �ʃp�����[�^ */
		MaxSecNum = AppServ_BinCsvSave.param_info.psection_num;	/* Max�Z�N�V������ get */
		MaxItemNum = AppServ_BinCsvSave.param_info.psection[SecNum].item_num;
															/* Max�A�C�e���� get */
	}

	/* CSV�擪�J�����̎w��f�[�^�i�Z�N�V�������C�A�C�e�����j�͈͊O�`�F�b�N */
	if( (SecNum >= MaxSecNum) || (ItemNum >= MaxItemNum) ){	/* �ԍ��I�[�o�[ (Y) */
/* �Z�N�V�������E�A�C�e�������قȂ����ꍇ�A�f�[�^�Z�b�g���Ȃ��Ő���I�� */
		return	(uchar)1;									/* ����I�� */
	}

	/** ��M�f�[�^��Hex�ϊ������[�N�G���A�Ɋi�[ **/

	/* �ړI�Z�N�^�[�̐擪�A�C�e���A�h���Xget */
	TgtSecTopIndex = AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( f_CParam, SecNum );
						/* �p�����[�^�G���A�擪����ړI�Z�N�^�[�擪�A�C�e���܂ł̑��A�C�e����get */
	pTgtSecTop = (ulong*)AppServ_BinCsvSave.Csv_toBin_Param.param_image;	/* work area top address get */
	pTgtSecTop += (TgtSecTopIndex + ItemNum);

	for( i=0; ; ++i ){
		/* �A�C�e�����͈͊O�i�I�[�o�[�j�`�F�b�N */
		if( (ItemNum+i) >= MaxItemNum ){					/* �A�C�e�����I�[�o�[ (Y) */
			break;											/* �����I��(����) */
		}

		ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
															/* 1�J�����f�[�^�ϊ� */
		if( 0 != (ret & (BIT_COMMENT | BIT_ERROR)) ){		/* �G���[or�R�����g(Y) */
			*finish = 1;
			return	(uchar)0;								/* �ُ�I�� */
		}

		if( 0 != (ret & BIT_DATA_EXIST) ){					/* �f�[�^���� (Y) */
			pTgtSecTop[i] = BinData;						/* ���[�N�G���A�Ƀf�[�^�Z�b�g */
			prm_invalid_change( (short)SecNum, (short)(ItemNum+i), (long*)&pTgtSecTop[i] );
		}	/* �f�[�^�Ȃ����͂O�Ƃ��邪�A���������ɂO�N���A���Ă���̂ł����ł͉������Ȃ� */

		if( 0 == (ret & BIT_NEXT_EXIST) ){					/* ���f�[�^�Ȃ� (Y) */
			break;											/* ����I�� */
		}
	}
	return	(uchar)1;										/* normal �I�� */
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�_�E�����[�h �T�u���[�`��							|*/
/*|																			|*/
/*|	CSV�`���f�[�^�̂P�J������HEX�f�[�^�ɕϊ��B								|*/
/*|	","(�J���})�ŋ�؂�ꂽ�f�[�^�P�̉�͂ƕϊ����s���B					|*/
/*|	�f�[�^���Z�N�V�����ԍ��������R�����g�s�ł���ꍇ�́A���̏���Ԃ��B	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PC�����M����CSV�`���f�[�^					|*/
/*|				   pBinData = HEX�ϊ��l�Z�b�g�G���A�ւ̃|�C���^				|*/
/*|							  return�l=1���̂ݗL��							|*/
/*|				   pNextCsvData = ���f�[�^�̐擪�A�h���X					|*/
/*| RETURN VALUE : 0=�f�[�^�Ȃ��A1=���l�f�[�^�L��ipBinData�ɃZ�b�g�j		|*/
/*|				   2=�R�����g�s�i�ϊ��Ȃ��j                                 |*/
/*|				   b0=1:BinData����i�ϊ����{�j								|*/
/*|				   b1=1:���f�[�^����A0:�ŏI�f�[�^							|*/
/*|				   b2=1:�R�����g�J����(�擪)�A0:�R�����g�łȂ�				|*/
/*|				   b7=1:�f�[�^�G���[����									|*/
/*|			�i��j															|*/
/*|				   06h=�R�����g�J�����̂��߃f�[�^�Ȃ�  ���f�[�^����			|*/
/*|				   03h=���l�f�[�^�L��(pBinData�ɃZ�b�g)���f�[�^����			|*/
/*|				   01h=���l�f�[�^�L��(pBinData�ɃZ�b�g)���f�[�^�Ȃ�(�ŏI)	|*/
/*|				   04h=�R�����g�J�����̂��߃f�[�^�Ȃ�  ���f�[�^�Ȃ�(�ŏI)	|*/
/*|				   80h=�`���G���[�i�ϊ��Ȃ��j								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvCsvParam_1ColumnRead( const char *CsvData, ulong *pBinData, const char **pNextCsvData )
{
	char	Asc[12];
	ushort	i,j;											/* i= read index */
															/* j= data get count */
	char	TgtChar;
	uchar	ret;
	ulong	BinData;

	ret = 0;

	for( i=0, j=0; j<11; ++i ){								/* �ő�10�����擾����܂� */
		TgtChar = CsvData[i];
		if( ' ' == TgtChar ){								/* �X�y�[�X�͓ǂݔ�΂� */
			continue;
		}

		if( ',' == TgtChar ){								/* ��؂蕶��(������) */
			*pNextCsvData = (char*)&CsvData[i+1];					/* ���A�h���X�Z�b�g */
			ret |= BIT_NEXT_EXIST;
			goto AppServ_CnvCsvParam_1ColumnRead_10;		/* 1�J�������o���� ������ */
		}

		else if( (0x0d == TgtChar) ||						/* �s�����R�[�h */
				 (0x0a == TgtChar) ||
				 (0x1a == TgtChar) ){

AppServ_CnvCsvParam_1ColumnRead_10:
			/** 1�J�������o���� ���� **/
			if( 0 == j ){									/* �f�[�^�Ȃ�(Y) */
				//break;
			}
			else if( '[' == Asc[0] ){						/* �R�����g�s(Y) */
				ret |= BIT_COMMENT;
				//break;
			}
			else{
				if( 1 == DecAsc_to_LongBin_withCheck((uchar*)&Asc[0], (uchar)j, &BinData) ){
															/* Ascii -> Hex�ϊ� */
					*pBinData = BinData;					/* Hex data set */
					ret |= BIT_DATA_EXIST;					/* �f�[�^���� */
				}
				else{										/* ���f�[�^�ُ� */
					ret |= BIT_ERROR;
				}
				//break;
			}
			break;
		}

		else{												/* �I���R�[�h�ł͂Ȃ� */
			Asc[j++] = TgtChar;
			if( 10 < j ){									/* �f�[�^�傫����error */
				ret |= BIT_ERROR;
				break;
			}
		}
	}
	return	ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�_�E�����[�h �T�u���[�`��							|*/
/*|																			|*/
/*|	CSV�`���f�[�^�̂P�J�����ڂ̃R�����g����^�[�Q�b�g�Z�N�V�����ԍ���		|*/
/*|	�A�C�e���ԍ��𓾂�B													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PC�����M����CSV�`���f�[�^					|*/
/*|				   pCParam = 1:���ʃp�����[�^�A0=�ʃp�����[�^				|*/
/*|				   pSecNum = �Z�N�^�[�ԍ� (1�`)								|*/
/*|				   pItemNum = �A�C�e���ԍ� (0�`)							|*/
/*| RETURN VALUE : 0=OK, 80=error											|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_CnvCsvParam_SecItemNumGet( const char *CsvData, uchar *pCParam, ushort *pSecNum, ushort *pItemNum )
{
	char	Asc[12];
	ushort	i,j;											/* i= read index */
															/* j= data get count */
	char	TgtChar;
	uchar	ret;
	ulong	BinData;

	ret = 0;
	memset(Asc,0,sizeof(Asc));

	for( i=0, j=0; j<11; ++i ){								/* �ő�10�����擾����܂� */
		TgtChar = CsvData[i];
		if( ' ' == TgtChar ){								/* �X�y�[�X�͓ǂݔ�΂� */
			continue;
		}

		if( ',' == TgtChar){								/* ��؂蕶��(Y) */

			/** 1�J�������o���� ���� **/
			/* �f�[�^�`���� "[Css-iiii]" or "[Pss-iiii]" �����󂯕t���� */
			if( ('[' != Asc[0]) || ('-' != Asc[4]) || (']' != Asc[9]) ){	/* �`���G���[(Y) */
				return	(uchar)BIT_ERROR;
			}

			if( 'C' == Asc[1] ){
				*pCParam = 1;
			}
			else if( 'P' == Asc[1] ){
				*pCParam = 0;
			}
			else{
				return	(uchar)BIT_ERROR;
			}

			/* �Z�N�V�����ԍ��𓾂� */
			if( 0 == DeciAsc_to_LongBin_withCheck((uchar*)&Asc[2], (uchar)2, &BinData) ){
															/* 10�i���� -> Hex�ϊ� */
				return	(uchar)BIT_ERROR;
			}
			*pSecNum = (ushort)BinData;

			/* �擪�A�C�e���ԍ��𓾂� */
			if( 0 == DeciAsc_to_LongBin_withCheck((uchar*)&Asc[5], (uchar)4, &BinData) ){
															/* 10�i���� -> Hex�ϊ� */
				return	(uchar)BIT_ERROR;
			}
			*pItemNum = (ushort)BinData;
			break;
		}

		else{												/* �I���R�[�h�ł͂Ȃ� */
			Asc[j++] = TgtChar;
			if( 10 < j ){									/* �f�[�^�傫����error */
				ret |= BIT_ERROR;
				break;
			}
		}
	}
	return	ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�_�E�����[�h �T�u���[�`��							|*/
/*|																			|*/
/*|	�p�����[�^�G���A�擪����ړI�Z�N�^�[�̐擪Item�܂ł̑��A�C�e�����擾	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : SecKind = 1:���ʃp�����[�^�A0:�ʃp�����[�^				|*/
/*|				   SecNum = �Z�N�^�[�ԍ� (0�`)								|*/
/*| RETURN VALUE : �p�����[�^�G���A�擪����̑�Item��						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( char SecKind, ushort SecNum )
{
	ushort	SkipItemCount;
	ushort	i;

	SkipItemCount = 0;
	for( i=0; i<SecNum; ++i ){
		if( 1 == SecKind ){
			SkipItemCount += AppServ_BinCsvSave.param_info.csection[i].item_num;
		}else{
			SkipItemCount += AppServ_BinCsvSave.param_info.psection[i].item_num;
		}
	}

	if( 0 == SecKind ){					/* �ʃp�����[�^ (Y) */

		/* ���ʃp�����[�^�̑S�A�C�e�������X�L�b�v���Ƃ��ĎZ�o */
		SecNum = AppServ_BinCsvSave.param_info.csection_num;	/* ���ʃp�����[�^�Z�N�V������get */
		for( i=0; i<SecNum; ++i ){
			SkipItemCount += AppServ_BinCsvSave.param_info.csection[i].item_num;
		}
	}
	return	SkipItemCount;
}

/*[]----------------------------------------------------------------------[]*/
/*| Calculate SUM (unsigned short���̉��Z�B���ʂ�(unsigned short)	  	   |*/
/*|																		   |*/
/*|	�{�֐��ɂ͈ȉ��̏���������B�K���������������Call���邱�ƁB		   |*/
/*|	�@ �����O�X�͋������ł��邱�ƁB										   |*/
/*| �A ���Z���G���A�̐擪�A�h���X�͋����Ԓn�ł��邱�ƁB					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : kind - SUM�Z�o�Ώۃf�[�^(OPE_DTNUM_XXX)				   |*/
/*| RETURN VALUE : none						                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 202006-02-20                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	DataSumUpdate( ushort kind )
{
	ushort 	Sum;
	union	{
		ushort	us[2];
		ulong	ul;
	} u;
	
	if (AppServ_SumAreaInfoTbl[kind].address != NULL) {
		Sum = sumcal_ushort((ushort*)AppServ_SumAreaInfoTbl[kind].address, 
						(ushort)AppServ_SumAreaInfoTbl[kind].size);	/* sum���Z */
		u.us[0] = Sum;
		u.us[1] = ~Sum;
		DataSum[kind].Len = AppServ_SumAreaInfoTbl[kind].size;
		DataSum[kind].Sum = u.ul;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| RAM��f�[�^���j�����Ă��邩�ۂ��̃`�F�b�N			   |*/
/*|																		   |*/
/*| �T���`�F�b�N���s���Ó����󋵂�߂��B								   |*/
/*|	�{�֐��ɂ͈ȉ��̏���������B�K���������������Call���邱�ƁB		   |*/
/*|	�@ �����O�X�͋������ł��邱�ƁB										   |*/
/*| �A ���Z���G���A�̐擪�A�h���X�͋����Ԓn�ł��邱�ƁB					   |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : kind - SUM�Z�o�Ώۃf�[�^(OPE_DTNUM_XXX)				   |*/
/*| RETURN VALUE : 1=OK, 0=NG(�T��NG)							           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida kei                                             |*/
/*| Date         : 202006-02-20                                              |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	DataSumCheck( ushort kind )
{
	ushort 	Sum;
	union	{
		ushort	us[2];
		ulong	ul;
	} u;

	if (AppServ_SumAreaInfoTbl[kind].address == NULL) {
		return 0;	/* SUM�������Ȃ��f�[�^ */
	}

	if (DataSum[kind].Len != AppServ_SumAreaInfoTbl[kind].size) {
		return 0;	/* Sum NG(�f�[�^���̑���) */
	}
	
	Sum = sumcal_ushort((ushort*)AppServ_SumAreaInfoTbl[kind].address, 
						(ushort)AppServ_SumAreaInfoTbl[kind].size);	/* sum���Z */

	u.ul = DataSum[kind].Sum;
	if( (u.us[0] == Sum) && (u.us[1] == (ushort)(~Sum)) ){
		return	(uchar)1;						/* OK */
	}
	return	(uchar)0;								/* Sum NG(�f�[�^�j��) */
}

#define	PRMSUM_RAM_OK		0x0001
#define	PRMSUM_RAM_NG		0x0002
#define	PRMSUM_FLASH_OK		0x0010
#define	PRMSUM_FLASH_NG		0x0020
#define	PRMSUM_EQUAL		0x0100
#define	PRMSUM_NOT_EQUAL	0x0200
#define	PRM_COPY_FtoRAM		0x4000
/*[]-----------------------------------------------------------------------[]*/
/*|	FlashROM�֘A ���d����													|*/
/*|																			|*/
/*|	���d�ƃ������[�N���A�𕹗p���������B									|*/
/*|	���d����Phase��opetask����Call����邪�A���̎��_�Ń������[�N���A�����{	|*/
/*|	����B																	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : RAMCLR : �����N���A�w���t���O (1=�����A0=�����łȂ�)		|*/
/*|				   f_CPrmDefaultSet : 1=���ɋ������Ұ���default���(������)�ς�|*/
/*|				                      0=���������Ă��Ȃ�					|*/
/*| RETURN VALUE : none														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : machida.k												|*/
/*| Date         : 2005/07/26												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_FukudenProc( char RAMCLR, char f_CPrmDefaultSet )
{
	uchar	param_ok;
	ulong	ret;
	uchar	lockparam_ok;
	ulong	ver_flg;
	ushort	sntp_prm_after;		// SNTP���������ύX���j�^���O�o�^�p(�ݒ�34-0122�̇B�C�D�E��ێ�)
	ushort	i;

	/* �������[�N�G���A������ */
	for (i = 0; i < LOG_STRAGEN; i++) {
		AppServ_LogWriteCompleteCheckParam[i] = 0xffffffff;	/* LOG Flash�����݊����`�F�b�N�p �p�����[�^ */
	}

	FLAGPT.event_CtrlBitData.BIT.PARAM_LOG_REC = 0;				// ���O�o�^�\��Ȃ�
	FLAGPT.event_CtrlBitData.BIT.PARAM_INVALID_CHK = 1;			// �Œ�p�����[�^�ϊ����N�����̏������ǂ����𔻒肷��
/* �t���b�V���ǂݏ������W���[�������� */
	FLT_init(&param_ok, &lockparam_ok, &ver_flg, RAMCLR);

	if( f_ParaUpdate.BYTE != 0 ) {
		if( f_ParaUpdate.BIT.splday == 1 ) {
			UsMnt_datecheck(0);							// ���ʓ��^���ʊ��Ԃ̓��͓��t�̃`�F�b�N
		}
		if( f_ParaUpdate.BIT.tickvalid == 1 ) {
			UsMnt_datecheck(1);							// �������̓��͓��t�̃`�F�b�N
			memset( &tick_valid_data.tic_prm, 0, sizeof(TIC_PRM) );
		}

		DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
		if( f_ParaUpdate.BIT.bpara == 1 ) {				// ���ʃp�����[�^�ݒ�ύX����
			wopelg( OPLOG_KYOTUPARAWT, 0, 0 );			// ���엚��o�^
			sntp_prm_after = (ushort)prm_get(COM_PRM, S_NTN, 122, 4, 1);
			if (sntp_prm_before != sntp_prm_after) {
			// SNTP���������ݒ�l�ɕύX���������ꍇ�AM8036�o�^
				wopelg( OPLOG_SET_SNTP_SYNCHROTIME, (ulong)sntp_prm_before, (ulong)sntp_prm_after);
			}
			SetSetDiff(SETDIFFLOG_SYU_SYSMNT);			// �ݒ�X�V�������O�o�^
		}
		if( f_ParaUpdate.BIT.cpara == 1 ) {				// �Ԏ��p�����[�^�ݒ�ύX����
			DataSumUpdate(OPE_DTNUM_LOCKINFO);			// SUM�X�V
			wopelg( OPLOG_SHASHITUPARAWT, 0, 0 );		// ���엚��o�^
		}
		f_ParaUpdate.BYTE = 0;
	}

	if(( TENKEY_F1 == 1 )&&( TENKEY_F2 == 1 )&&( TENKEY_F3 == 1 )){		// ۯ����u���ؽı�v��(F1&F2&F3 ON)?
	/* FLAPDT���X�g�A */
		if( !( FLT_Restore_FLAPDT() & 0xff000000 ) ){
			grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[66] );						// "       ۯ����u���ؽı        ",�\��
		}
	}
	if (ver_flg) {
	/* �o�[�W�����A�b�v�w�肠��Ȃ̂ŁA�f�t�H���g�l�Z�b�g��A�S���X�g�A���s */
		log_init();
		RAMCLR = memclr( 1 );				// SRAM�������i�������ر�j
		prm_clr( PEC_PRM, 1, 0 );			// �����Ұ���̫��
		prm_clr( COM_PRM, 1, 0 );			// �������Ұ���̫��
		lockinfo_clr(1);					// �Ԏ����Ұ���̫��
		ret = FLT_Restore(FLT_EXCLUSIVE);	// ���X�g�A���s
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
			DataSumUpdate(OPE_DTNUM_LOCKINFO);	/* update parameter sum on ram */
		}
	}

// �����Ұ���߰�����ύX
	AppServ_PParam_Copy = OFF;							// FLASH �� RAM �����Ұ���߰�L������߰���Ȃ�
	
/* �p�����[�^�f�[�^���������� */
	PowonSts_Param = 0;			/* �N�����p�����[�^��ԃZ�[�u�G���A�������i�`�F�b�N�����{�j*/
	
	/* RAM��p�����[�^�f�[�^�̗L��/�����`�F�b�N */
	if (DataSumCheck(OPE_DTNUM_COMPARA) == 0) {
		PowonSts_Param |= PRMSUM_RAM_NG;
	}else{
		PowonSts_Param |= PRMSUM_RAM_OK;
	}
	
	/* FLASH��̃p�����[�^�f�[�^�L��/�����`�F�b�N */
	if (param_ok) {
		PowonSts_Param |= PRMSUM_FLASH_OK;
		/** ��۸����ް�ޮݱ��߂ɂ��A�������Ұ��̻��ނ��傫���Ȃ����ꍇ�̑΍� 	**/
		/** FlashROM���̾���ݐ��A�e����ݓ��̱��ѐ��� ����۸���(SRAM)�ƈقȂ�ꍇ**/
		/** �@ SRAM�̋������Ұ��ر����̫�ĸر����B								**/
		/** �A FlashROM���ɂ��镪���ް���SRAM�֏������ށB						**/
		/**    �����̎� �������������ް��͍X�V���Ȃ��B							**/
		/** �B �VSRAM���e��Master�Ƃ��AFlashROM���ް����������ށB				**/
		/** �� ���̏�������۸����ް�ޮݱ��߂��A�������Ұ��̻��ނ��傫���Ȃ���	**/
		/**    �ꍇ�̂݋@�\����B												**/
		/**    �A��F2&F4�CF3&F5�����N�����͋�����̫�Ă�D�悵Flash���e�͔p������**/

		if( 2 == FLT_Comp_ComParaSize_FRom_to_SRam() ){		// ����ݐ� ���� ���ѐ����Ⴄ

			/* ���Ұ������������ꍇ�;���ݐ����قȂ��Ă�Flash��SRAM�͂��Ȃ� */
			if( f_CPrmDefaultSet ){								// ���Ұ�����������
				(void)FLT_WriteParam1(FLT_EXCLUSIVE);			// RAM��FLASH�ւ̃p�����[�^�f�[�^�]��
			}
			else{
				prm_clr( COM_PRM, 1, 0 );						// �������Ұ���̫��set
				FLT_RestoreParam();								// FLASH��RAM�ւ̃p�����[�^�f�[�^�]��
				DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
																// �����Ұ��ͺ�߰���Ȃ��iAppServ_PParam_Copy = OFF�j
				(void)FLT_WriteParam1(FLT_EXCLUSIVE);			// RAM��FLASH�ւ̃p�����[�^�f�[�^�]��
			}
		}
	}
	else {
		PowonSts_Param |= PRMSUM_FLASH_NG;
	}
	
	if (PowonSts_Param & PRMSUM_RAM_OK) {
		/* RAM��FLASH�ւ̃p�����[�^�f�[�^�]�� */
		ret = FLT_WriteParam1(FLT_EXCLUSIVE);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			if ((PowonSts_Param & PRMSUM_RAM_OK)
				&& (PowonSts_Param & PRMSUM_FLASH_OK)) {
				if (_FLT_RtnDetail(ret) == FLT_PARAM_WRITTEN) {
					PowonSts_Param |= PRMSUM_NOT_EQUAL;	/* FLASH!=RAM */
				}
				else {
					PowonSts_Param |= PRMSUM_EQUAL;		/* FLASH==RAM */
				}
			}
		}
		else {
			PowonSts_Param &= ~(PRMSUM_FLASH_OK);
			PowonSts_Param |= PRMSUM_FLASH_NG;			/* �����ݎ��s */
		}
	}
	else if (PowonSts_Param & PRMSUM_FLASH_OK) {
		/* FLASH��RAM�ւ̃p�����[�^�f�[�^�]�� */
		FLT_RestoreParam();
		DataSumUpdate(OPE_DTNUM_COMPARA);	/* update parameter sum on ram */
		PowonSts_Param |= PRM_COPY_FtoRAM;	/* FLASH��RAM�ւ̃p�����[�^�f�[�^�]�� */
	}
	else {
		/* FLASH=NG && RAM=NG �̃P�[�X */
		/* AMANO�a�w���ɂ��f�t�H���g�l�̃Z�b�g���͂����ɂ��̂܂܋N��
		  (�����Ƃ��Ă��肦�Ȃ��P�[�X�Ƃ̂���) */
	}

/* �Ԏ��p�����[�^�[���������� */
	PowonSts_LockParam = 0;			/* �N�����p�����[�^��ԃZ�[�u�G���A�������i�`�F�b�N�����{�j*/
	
	/* RAM��p�����[�^�f�[�^�̗L��/�����`�F�b�N */
	if (DataSumCheck(OPE_DTNUM_LOCKINFO) == 0) {
		PowonSts_LockParam |= PRMSUM_RAM_NG;
	}else{
		PowonSts_LockParam |= PRMSUM_RAM_OK;
	}
	
	/* FLASH��̃p�����[�^�f�[�^�L��/�����`�F�b�N */
	if (lockparam_ok) {
		PowonSts_LockParam |= PRMSUM_FLASH_OK;
	}
	else {
		PowonSts_LockParam |= PRMSUM_FLASH_NG;
	}
	
	if (PowonSts_LockParam & PRMSUM_RAM_OK) {
		/* RAM��FLASH�ւ̃p�����[�^�f�[�^�]�� */
		ret = FLT_WriteLockParam1(FLT_EXCLUSIVE);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			if ((PowonSts_LockParam & PRMSUM_RAM_OK)
				&& (PowonSts_LockParam & PRMSUM_FLASH_OK)) {
				if (_FLT_RtnDetail(ret) == FLT_PARAM_WRITTEN) {
					PowonSts_LockParam |= PRMSUM_NOT_EQUAL;	/* FLASH!=RAM */
				}
				else {
					PowonSts_LockParam |= PRMSUM_EQUAL;		/* FLASH==RAM */
				}
			}
		}
		else {
			PowonSts_LockParam &= ~(PRMSUM_FLASH_OK);
			PowonSts_LockParam |= PRMSUM_FLASH_NG;			/* �����ݎ��s */
		}
	}
	else if (PowonSts_LockParam & PRMSUM_FLASH_OK) {
		/* FLASH��RAM�ւ̃p�����[�^�f�[�^�]�� */
		FLT_RestoreLockParam();
		DataSumUpdate(OPE_DTNUM_LOCKINFO);		/* RAM��p�����[�^�f�[�^��SUM�X�V */
		PowonSts_LockParam |= PRM_COPY_FtoRAM;	/* FLASH��RAM�ւ̃p�����[�^�f�[�^�]�� */
	}
	else {
		/* FLASH=NG && RAM=NG �̃P�[�X */
		/* AMANO�a�w���ɂ��f�t�H���g�l�̃Z�b�g���͂����ɂ��̂܂܋N��
		  (�����Ƃ��Ă��肦�Ȃ��P�[�X�Ƃ̂���) */
	}
	if( prm_invalid_check() ){
		FLT_WriteParam1(FLT_EXCLUSIVE);					// RAM��FLASH�ւ̃p�����[�^�f�[�^�]��
		DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
	}

}

//[]----------------------------------------------------------------------[]
///	@brief		LOG Flash�����݊J�n����
///
/// FlashROM�^�X�N��RAM��̐��ZLOG��FlashROM�֏������ނ悤�v������B
/// PowerON��̕��d��������Call���ꂽ�ꍇ�̓^�X�N�ؑւ����Ȃ������݂ŗv���B
/// �����łȂ��ꍇ�̓^�X�N�ؑւ��s���Ȃ��珑�����݂��s�������ŗv������B
/// ���̏ꍇ �����݂̊����� AppServ_IsSaleLogFlashWriting() �Ŋm�F���邱�ƁB
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void AppServ_LogFlashWriteReq( short id )
{
	uchar	f_FlashWriteMode;
	ulong	ret_ul;
	uchar	*rec;
	short	num = LOG_DAT[id].count[LOG_DAT[id].kind];
	short	kind = LOG_DAT[id].kind;

	if( LogDatMax[id][0]>RECODE_SIZE ){
		rec = LOG_DAT[id].dat.s2;
	}
	else{
		rec = LOG_DAT[id].dat.s1[LOG_DAT[id].kind];
	}

	if( num==0 ){	/* LOG�f�[�^�Ȃ� */
		return;
	}

	if (AppServ_IsLogFlashWriting(id) != 0) {
		return;		// ���Ɏ��s���Ȃ���Ȃ�
	}

	if( FLT_f_TaskStart==0 ){						/* FlashROM�^�X�N�N���O�i���d�����Ȃǁj*/
		f_FlashWriteMode = FLT_EXCLUSIVE;			/* �^�X�N�ؑւȂ��i��C�����݁j���[�h */
	}
	else{
		f_FlashWriteMode = FLT_NOT_EXCLUSIVE;		/* �^�X�N�ؑւ��胂�[�h */
	}

	/* LOG�����ݗv�� */
	ret_ul = FLT_WriteLog(id, (const char*)rec, num, f_FlashWriteMode, kind);

	switch( _FLT_RtnKind(ret_ul) ){
	case FLT_NORMAL:
		if( f_FlashWriteMode==FLT_NOT_EXCLUSIVE ){	/* �^�X�N�ؑւ��胂�[�h */
			/* �����`�F�b�N���ɕK�v�ȃp�����[�^�ۑ� */
			AppServ_LogWriteCompleteCheckParam[id] = ret_ul;
			return;
		}
		else{
			/* �v�����ł͂Ȃ����Z�b�g */
			AppServ_LogWriteCompleteCheckParam[id] = 0xffffffff;
		}
		break;
	case FLT_ERASE_ERR:	/* �������ݎ��s���̓���������ōs�� */
	case FLT_WRITE_ERR:
	case FLT_VERIFY_ERR:
	case FLT_PARAM_ERR:
	case FLT_BUSY:
		/* �v�����ł͂Ȃ����Z�b�g */
		AppServ_LogWriteCompleteCheckParam[id] = 0xffffffff;
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG Flash�����ݏI���`�F�b�N����
///
/// FlashROM��RAM��̐��ZLOG���������ݒ����ۂ��m�F����B
/// OPE�͐ڋq�J�n�O�ɖ{�֐���Call���ď������ݒ��ł���ΐڋq���J�n���Ȃ�
/// �l�ɂ���B
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG���
///	@return		1=�����ݒ��A0=�����ݒ��ł͂Ȃ��i�I�����܂ށj
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar AppServ_IsLogFlashWriting( short Lno )
{
	uchar	ret_uc;
	ulong	ret_ul;

	if( AppServ_LogWriteCompleteCheckParam[Lno]==0xffffffff ){	// �v�����Ă��Ȃ�
		return	0;
	}

	/* �����݊����`�F�b�N */
	ret_uc = FLT_ChkWriteCmp( AppServ_LogWriteCompleteCheckParam[Lno], &ret_ul);
	if( ret_uc==0 ){		// ������
		return	1;
	}

	/* �����݊����i�^�X�N�ؑփ��[�h�ł̗v���I���j*/
	AppServ_LogWriteCompleteCheckParam[Lno] = 0xffffffff;	// �v�����ł͂Ȃ����Z�b�g

	switch( _FLT_RtnKind(ret_ul) ){		// �X�e�[�^�X�`�F�b�N
	case	FLT_NORMAL:
		break;
	case	FLT_ERASE_ERR:	/* �������ݎ��s���̓���������ōs�� */
	case	FLT_WRITE_ERR:
	case	FLT_VERIFY_ERR:
	case	FLT_PARAM_ERR:
	case	FLT_BUSY:
		break;
	}

	return	0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_ConvLockInfo2CSV_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��f�[�^�A�b�v���[�h�������֐�
 *| param	: void
 *| return	: 1:����������
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvLockInfo2CSV_Init( void )
{
	AppServ_B2C_LockInfo.CallCount = 0;

	return 1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvLockInfo2CSV()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��f�[�^�ϊ��֐��ibin�@���@csv�j
 *| 		: �P���̎Ԏ��f�[�^���P�s��csv�`���̕�����ɕϊ�����B
 *| param	: csvdata		�ϊ���f�[�^�̊i�[��
 *| param	: csvdata_len	�ϊ���f�[�^�̒���
 *| param	: finish		�ϊ��̏I���t���O�i��������R�Q�S��j
 *| return	: void
 *[]----------------------------------------------------------------------[]*/
void	AppServ_CnvLockInfo2CSV( char *csvdata, ushort *csvdata_len, uchar *finish )
{
	ushort cnv_len;
	cnv_len = 0;
	// ���b�N���u���
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].lok_syu, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// �������
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].ryo_syu, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// ���
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].area, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// ���Ԉʒu�i���o�[
	cnv_len += AppServ_CnvLockInfo2CSV_SUB(LockInfo[AppServ_B2C_LockInfo.CallCount].posi, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// �eI/F�Ճi���o�[
	cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].if_oya, &csvdata[cnv_len], SEPARATOR_COMMA);
	
	// �eI/F�łɑ΂��鑕�u�A��
	if( AppServ_B2C_LockInfo.CallCount < BIKE_START_INDEX ) {	// ���ԏ��̏ꍇ�A�ϊ����K�v
		cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)( LockInfo[AppServ_B2C_LockInfo.CallCount].lok_no + PRM_LOKNO_MIN ), &csvdata[cnv_len], SEPARATOR_NEW_LINE);	// �\���p��+100�����l���킽��
	} else {													// ���ւ͏]���ʂ�
		cnv_len += AppServ_CnvLockInfo2CSV_SUB((ulong)LockInfo[AppServ_B2C_LockInfo.CallCount].lok_no , &csvdata[cnv_len], SEPARATOR_NEW_LINE);
	}

	// �ϊ���̒�����ݒ�B
	*csvdata_len = cnv_len;

	// �Ăяo���񐔃C���N�������g
	AppServ_B2C_LockInfo.CallCount++;

	// �I���t���O�ݒ�
	if (AppServ_B2C_LockInfo.CallCount >= ITEM_MAX_LOCK_INFO) {
		*finish = 1;
	} else {
		*finish = 0;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvLockInfo2CSV_SUB()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��f�[�^�ϊ��֐��ibin�@���@csv�j
 *| 		: �����ŗ^����ꂽ�e�p�����[�^���P�O�iASCII������ɕϊ�����B
 *| param	: param		�ϊ�����p�����[�^
 *| param	: buf		�ϊ���̕�������[�U����o�b�t�@�̐擪
 *| param	: separator	�ϊ���̕�����̍Ō�ɕt������Z�p���[�^
 *| return	: uchar		�ϊ�����������̒���
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvLockInfo2CSV_SUB(ulong param, char *buf, uchar separator)
{
	uchar cnv_len;

	cnv_len = intoasl_0sup((uchar*)buf, param, (ushort)10 );

	if (separator == SEPARATOR_COMMA) {
		buf[cnv_len] = ',';
		cnv_len++;
	} else {
		buf[cnv_len]		= 0x0d;
		buf[cnv_len + 1]	= 0x0a;
		cnv_len += 2;
	}

	return cnv_len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_ConvCSV2LockInfo_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��f�[�^�A�b�v���[�h�������֐��icsv�@���@bin�j
 *| param	: lock_info_image	�ϊ���Ԏ��f�[�^�i�[�惏�[�N�G���A
 *| param	: image_size		���[�N�G���A�T�C�Y
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSV2LockInfo_Init( char *lock_info_image, ulong image_size )
{

	if (image_size < sizeof(LockInfo)) {
		return	(uchar)0;
	}

	AppServ_C2B_LockInfo.lock_info_image = (t_LockInfo*)lock_info_image;
	AppServ_C2B_LockInfo.image_size = image_size;
	AppServ_C2B_LockInfo.CallCount = 0;

	memset(lock_info_image, 0, sizeof(LockInfo));

	return	(uchar)1;

}
/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvCSV2LockInfo()
 *[]----------------------------------------------------------------------[]
 *| summary	: 
 *| param	: csvdata		�ϊ����f�[�^
 *| param	: csvdata_len	�ϊ����f�[�^�T�C�Y
 *| param	: finish		�ϊ��I���̃t���O�i��������R�Q�S��j
 *| return	: 1:OK	0:NG
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSV2LockInfo(const char *csvdata, ushort csvdata_len, ulong *finish )
{
	uchar	ret;
	ulong	BinData;
	const char	*pNextCsvData;

	if (AppServ_C2B_LockInfo.CallCount >= ITEM_MAX_LOCK_INFO) {
		if (csvdata[0] == '\r') {
			*finish = 1;
			return 1;
		}
		else {
			return 0;	/* �ŏI�s�ȍ~�Ƀf�[�^������ */
		}
	}
	
	pNextCsvData = csvdata;

	/* ���b�N���u��� */
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, lok_syu)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_syu = (uchar)BinData;
	} else {
		return 0;
	}

	// �������
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, ryo_syu)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].ryo_syu = (uchar)BinData;
	} else {
		return 0;
	}

	// ���
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, area)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].area = (uchar)BinData;
	} else {
		return 0;
	}

	// ���Ԉʒu�i���o�[
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, posi)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].posi = BinData;
	} else {
		return 0;
	}

	// �eI/F�Ճi���o�[
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		if (!_appserv_CheckVal(BinData, t_LockInfo, if_oya)) {
			return 0;
		}
		AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].if_oya = (uchar)BinData;
	} else {
		return 0;
	}

	// �eI/F�łɑ΂��鑕�u�A��
	ret = AppServ_CnvCsvParam_1ColumnRead( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x01) {
		if( AppServ_C2B_LockInfo.CallCount < BIKE_START_INDEX ) {	// ���ԏ��̏ꍇ�A�ϊ����K�v
			if( BinData < PRM_LOKNO_MIN || BinData > PRM_LOKNO_MAX ){	// �͈̓`�F�b�N( 100�`315 �L�� )
				// �͈͊O�̏ꍇ�́u0�v��������̂ŁA_appserv_CheckVal�͍s���K�v���Ȃ��B
				AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_no = 0;									// �͈͊O�̏ꍇ�A0(�ڑ������)��ۑ�
			} else {
				if (!_appserv_CheckVal(BinData- PRM_LOKNO_MIN, t_LockInfo, lok_no)) {
					return 0;
				}
				AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_no = (uchar)( BinData- PRM_LOKNO_MIN );	// �͈͓��̏ꍇ�A100�������ĕۑ�����(�ۑ��̈悪1byte�̂���)
			}
		} else {													// ���ւ͏]���ʂ�
			if (!_appserv_CheckVal(BinData, t_LockInfo, lok_no)) {
				return 0;
			}
			AppServ_C2B_LockInfo.lock_info_image[AppServ_C2B_LockInfo.CallCount].lok_no = (uchar)BinData;
		}
	} else {
		return 0;
	}
	// �Ăяo���񐔃C���N�������g
	AppServ_C2B_LockInfo.CallCount++;

	// �I���t���O�ݒ�
	if (AppServ_C2B_LockInfo.CallCount >= ITEM_MAX_LOCK_INFO) {
		*finish = 1;
	} else {
		*finish = 0;
	}

	return 1;

}


/*[]----------------------------------------------------------------------[]
 *|	name	: appserv_CheckVal()
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�T�C�Y���̒l�͈̓`�F�b�N
 *| param	: data - �l�͈̓`�F�b�N�ΏۂƂ���f�[�^
 *|			  size - data����\��̗̈�T�C�Y
 *| return	: 1 = OK
 *|			  0 = NG
 *[]----------------------------------------------------------------------[]*/
int	appserv_CheckVal(ulong data, size_t size)
{
	switch (size) {
	case sizeof(uchar):
		if (data & 0xFFFFFF00) {
			return 0;
		}
		else {
			return 1;
		}
		break;
	case sizeof(ushort):
		if (data & 0xFFFF0000) {
			return 0;
		}
		else {
			return 1;
		}
	case sizeof(ulong):
		return 1;
	default:
		return 0;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	�d�����`���R�}���h���M�f�[�^�t�H�[�}�b�g�쐬							|*/
/*|																			|*/
/*|	pc�ɃA�b�v���[�h����f�[�^�`���ɕϊ�����								|*/
/*|	�i���Z�@���f�[�^�ɉ��s�R�[�h(CRLF)��}������j							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : inserted = ���s�}����f�[�^�i�[�G���A�ւ̃|�C���^		|*/
/*|				   text = ���s�}�����f�[�^(���Z�@���f�[�^)�ւ̃|�C���^		|*/
/*| RETURN VALUE : �ϊ���̃f�[�^��										    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : suzuki													|*/
/*| Date         : 2006/11/08												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
ushort	AppServ_InsertCrlfToEdyAtCommand( char *inserted, const char text[6][20] )
{
	ushort	i, len;
	
	len = 0;
	for (i = 0; i < 6; i++) {
		/* copy 1 line */
		memcpy(&inserted[len], &text[i][0], 20);
		len += 20;
		/* insert cr & lf */
		inserted[len++] = '\r';
		inserted[len++] = '\n';
	}
	return len;
}
/*[]-----------------------------------------------------------------------[]*/
/*|	�d�����`���R�}���h��M�f�[�^�t�H�[�}�b�g�쐬							|*/
/*|																			|*/
/*|	PC����_�E�����[�h���ꂽ�f�[�^��FlashROM�i�[�`���ɕϊ�����B			|*/
/*|	�����f�[�^��ASCII or Shift-JIS�ɑΉ��BUnicode�͕s�B					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : formated = �ϊ���f�[�^�i�[�G���A�ւ̃|�C���^			|*/
/*|				   text = �ϊ����f�[�^(PC�����M�����f�[�^)�ւ̃|�C���^	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : suzuki													|*/
/*| Date         : 2006/11/08												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	AppServ_FormatEdyAtCommand( char formated[6][20], const char *text, ushort text_len )
{
	ushort	Line;
	ushort	Col;
	ushort	i;

	memset( &formated[0][0], ' ', 6*20 );			/* set area space clear */

	Line = 0;
	Col  = 0;
	for( i=0; i<text_len; ++i ){
		if( 0x0d == text[i] ){
			++Line;
			/* �s��Max�`�F�b�N */
			if( 6 <= Line ){
				return;
			}
			Col = 0;
			if( 0x0a == text[i+1] ){
				++i;
			}
			continue;
		}
		else{
			/* ������Max�`�F�b�N */
			if( 20 <= Col ){
				continue;
			}
			formated[Line][Col++] = text[i];
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�f�[�^�A�b�v���[�h���̃f�[�^�ϊ��i�������j
//[]----------------------------------------------------------------------[]
///	@param[in]		kind 0:�װ 1:�װ� 2:��� 3:�������
///	@return			void
///	@note			RAM���f�[�^��CSV�`���l�ɕϊ�����B���̂��߂̑O�����B<br>
///					findfirst, findnext�̂悤�ɍŏ��ɖ{�֐��A�ȍ~��<br>
///					AppServ_ConvErrArmCSV()�ŏ����Ǐo�����s���B<br>
///					�܂��A1�Z�N�V����(1�s)�̃f�[�^�T�C�Y��32768Byte�ȉ���<br>
///					���邱��(CR/LF�܂�)<br>
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV_Init( uchar kind )
{

	// �{���ABin_toCsv_Param�͋��ʃp�����[�^�̃A�b�v���[�h�p�̈悾���A���O�n�̃A�b�v���[�h�ɂ�
	// �K�v�ȃG���A�̂ݎg�p���ĕ��p����B
	ushort	From_kind = eLOG_ERROR;

	/* ��ƃG���A�N���A */
	memset( &AppServ_BinCsvSave.Bin_toCsv_Param, 0, sizeof( t_AppServ_Bin_toCsv_Param ));
	AppServ_BinCsvSave.Bin_toCsv_Param.Kind = kind;			// ��Ǝ�ʂ̕ێ�

	if( kind != 4 ){
		switch( kind ){
			case	0:
				From_kind = eLOG_ERROR;
				break;
			case	1:
				From_kind = eLOG_ALARM;
				break;
			case	2:
				From_kind = eLOG_MONITOR;
				break;
			case	3:
				From_kind = eLOG_OPERATE;
				break;
		}
		AppServ_BinCsvSave.Bin_toCsv_Param.datasize = Ope_Log_TotalCountGet( From_kind );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�f�[�^�A�b�v���[�h���̃f�[�^�ϊ�
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in/out]	csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in/out]	finish		: �����ݏI������G���A
///	@return			void
///	@attention		�{�֐�Call�O�� AppServ_ConvErrArmCSV_Init() ��Call���邱�ƁB<br>
///					�ŏ�����ǂݏo���Ȃ����ꍇ���A�������֐���Call���邱�ƁB<br>
///	@note			RAM���f�[�^��CSV�`���l�ɕϊ����A�o�b�t�@�ɃZ�b�g����B<br>
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV( char *csvdata, ulong *csvdata_len, uchar *finish )
{
	ulong	TotalLen = 0;
	t_AppServ_Bin_toCsv_Param *p = &AppServ_BinCsvSave.Bin_toCsv_Param;
	
	*finish = 0;
	if( 0 == p->Phase ){	/* �擪�s�i���o���j���M�t�F�[�Y (Y)	*/
		AppServ_ConvErrArmCSV_Sub1( csvdata, csvdata_len, p );	/* ���o���쐬 */
		p->Phase = 1;		/* ���t�F�[�Y�̓f�[�^���ϊ� */
		return;
	}

	/** �f�[�^���ϊ��t�F�[�Y **/
	AppServ_ConvErrArmCSV_Sub2( csvdata, &TotalLen, p );			/* �擪�J�������o���쐬 */

	/* 1�Z�N�V�����Z�b�g���� ���� �����݃o�b�t�@�T�C�YLimit�܂Ńf�[�^�Z�b�g */
	*finish = AppServ_ConvErrArmCSV_Sub3( csvdata, &TotalLen, p );
	*csvdata_len = TotalLen;
}

const char *ErrCsvTitle[] =	// �G���[���O
{
	"E",
	"�G���[�R�[�h",
	"����/����",
	"�����N����",
	"���x��",
	"�⑫���",
	"����",
	"����",
};

const char *ArmCsvTitle[] =	// �A���[�����O
{
	"A",
	"�A���[���R�[�h",
	"����/����",
	"�����N����",
	"���x��",
	"�⑫���",
	"����",
	"����",
};

const char *MoniCsvTitle[] = // ���j�^���O
{
	"R",
	"���j�^�[�R�[�h",
	"�����N����",
	"���x��",
	"���j�^�[���",
};

const char *OpeMoniCsvTitle[] =	// ���샂�j�^���O
{
	"M",
	"���샂�j�^�[�R�[�h",
	"�����N����",
	"���x��",
	"�ύX�O�f�[�^",
	"�ύX��f�[�^",
};

const char *CarInfoCsvTitle[] =	// ���샂�j�^���O
{
	""			,"�G���A��"	,"mode"		,"nstat"	,"ostat"	,
	"ryo_syu"	,"year"		,"mont"		,"date"		,"hour"		,
	"minu"		,"passwd"	,"uketuke"	,"u_year"	,"u_mont"	,
	"u_date"	,"u_hour"	,"u_minu"	,"bk_syu"	,"s_year"	,
	"s_mont"	,"s_date"	,"s_hour"	,"s_minu"	,"lag_to_in",
	"issue_cnt"	,"bk_wmai"	,"bk_wari"	,"bk_time"	,"bk_pst"	,
	"in_chk_cnt","timer"	,"car_fail"	,"Dummy1"	,"Dummy2"	,
	"Dummy3"	,"Dummy4"	,"Dummy5"	,"Dummy6"	,"Dummy7"	,
	"Dummy8"	,"Dummy9"	,"Dummy10"	,
};
static ulong	LockNumMax = LOCK_MAX;

typedef	void	(*EDITFUNC)(char *,	ulong *, t_AppServ_Bin_toCsv_Param * );
typedef struct{
	uchar		data_max;
	const char	**title_data;
	ulong		*data_count;
	EDITFUNC	EditFunc;
}CSV_EDIT_INFO;

CSV_EDIT_INFO csv_edit[] = 
	{
		{ 6,	ErrCsvTitle, 		&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvErrDataCreate 	},	// [0]�G���[�f�[�^
		{ 6,	ArmCsvTitle, 		&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvArmDataCreate 	},	// [1]�A���[���f�[�^
		{ 5,	MoniCsvTitle, 		&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvMoniDataCreate 	},	// [2]���j�^�f�[�^
		{ 6,	OpeMoniCsvTitle, 	&AppServ_BinCsvSave.Bin_toCsv_Param.datasize, AppServ_ConvOpeMoniDataCreate },	// [3]���샂�j�^
		{ 43,	CarInfoCsvTitle, 	(ulong*)&LockNumMax, 			AppServ_ConvCarInfoDataCreate},	// [4]�Ԏ����
	};
	
//[]----------------------------------------------------------------------[]
///	@brief			���O�f�[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub1�j
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in/out]	csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�ϊ����̊Ǘ��̈�
///	@return			void
///	@note			CSV�t�@�C���̂P�s�ڌ��o�����쐬����B<br>
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV_Sub1( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	ushort	i;
	ushort	TotalLen = 0;

	for( i=1; i<csv_edit[p->Kind].data_max; ++i ){										// ���o�������[�v
		sprintf( &csvdata[TotalLen], "[%s]%s",csv_edit[p->Kind].title_data[i],"," );	// ���o����������Z�b�g
		TotalLen += (ushort)(strlen(csv_edit[p->Kind].title_data[i])+3);				// �ҏW�T�C�Y���Z�b�g
	}

	csvdata[TotalLen] = 0x0d;			// CR���Z�b�g
	csvdata[TotalLen+1] = 0x0a;			// LF���Z�b�g

	*csvdata_len = TotalLen + 2;		// CR/LF�������Z
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�f�[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub2�j
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in/out]	csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�ϊ����̊Ǘ��̈�
///	@return			void
///	@note			�e�s�̐擪�i�P�J�����ځj���o���쐬����
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrArmCSV_Sub2( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	uchar	code[2]={0};
	
	memset( csvdata, 0, 256 );
	switch( p->Kind ){	// ��Ǝ��
		case 0:			// �G���[���O
			Ope_Log_1DataGet( eLOG_ERROR, p->Section, &FTP_buff );	
			memcpy( code, &((Err_log *)FTP_buff)->Errsyu, 2 );	// �G���[�R�[�h���Z�b�g
			break;
		case 1:			// �A���[�����O
			Ope_Log_1DataGet( eLOG_ALARM, p->Section, &FTP_buff );	
			memcpy( code, &((Arm_log *)FTP_buff)->Armsyu, 2 ); // �A���[���R�[�h���Z�b�g
			break;
		case 2:			// ���j�^���O
			Ope_Log_1DataGet( eLOG_MONITOR, p->Section, &FTP_buff );	
			memcpy( code, &((Mon_log *)FTP_buff)->MonKind, 2 ); // ���j�^�R�[�h���Z�b�g
			break;
		case 3:			// ���샂�j�^���O
			Ope_Log_1DataGet( eLOG_OPERATE, p->Section, &FTP_buff );	
			memcpy( code, &((Ope_log *)FTP_buff)->OpeKind, 2 ); // ���샂�j�^�R�[�h���Z�b�g
			break;
		case 4:			// �Ԏ����
			sprintf( csvdata, "[%03d]%s", p->Section,",");
			*csvdata_len = 6;
			return;
		default:
			return;
	}

	// �e�R�[�h��擪�Z���ɃZ�b�g
	sprintf( csvdata, "[%s%03d-%03d]%s", csv_edit[p->Kind].title_data[0],code[0],code[1],",");
	*csvdata_len = 11;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub3�j
//[]----------------------------------------------------------------------[]
///	@param[in/out]	csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in/out]	csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�ϊ����̊Ǘ��̈�
///	@return			: 1=�����ݏI���B 0=���I��
///	@note			�f�[�^�Z�b�g�iCSV�쐬�j����<br>
///					1�s���̃f�[�^���Z�b�g�B�i�P�Z�N�V�����̏I���܂Łj<br>
///					�����ɂ�CR,LF���Z�b�g����B
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/21
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	AppServ_ConvErrArmCSV_Sub3( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	
	switch( p->Kind ){				// ��Ǝ��
		case 0:						// �G���[���O
		case 1:						// �A���[�����O
		case 2:						// ���j�^���O
		case 3:						// ���샂�j�^���O
		case 4:						// �Ԏ����
			csv_edit[p->Kind].EditFunc( csvdata, csvdata_len, p );	// �ҏW�֐��R�[��
			break;
		default:
			return 1;				// ��L�ȊO�͕ҏW�Ȃ�
	}
	
	csvdata[*csvdata_len] = 0x0d;	// CR���Z�b�g
	csvdata[*csvdata_len+1] = 0x0a;	// LF���Z�b�g
	*csvdata_len += 2;				// CR/LF���̃T�C�Y�����Z

	// �I���v������
	++p->Section;					// ���̃Z�N�V������
	if( *csv_edit[p->Kind].data_count <= p->Section ){
		return	1;					// �S�I�� 
	}

	return	0;						// ���I��
}

//[]----------------------------------------------------------------------[]
///	@brief			�G���[���O�f�[�^�A�b�v���[�h���̃f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in]		csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�쐬���̊Ǘ��̈�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvErrDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	uchar	Asc[10]={0};
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	Err_log	*plog;	// �G���[�f�[�^�ҏW�|�C���^
	
	Ope_Log_1DataGet( eLOG_ERROR, p->Section, &FTP_buff );	
	plog = (Err_log *)FTP_buff;
	
	// ����/�������
	OccurReleaseDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Errdtc, p );

	// �����N�������
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// ���x��
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Errlev );

	// �⑫���
	if( plog->Errinf ){														// �⑫��񂠂�H
		if( plog->Errinf == 1 ){											// �A�X�L�[�f�[�^�H
			sprintf( &csvdata[*csvdata_len], "%s%s", plog->Errdat,"," );	// �⑫�����Z�b�g
			*csvdata_len += (ushort)(strlen( (char*)plog->Errdat )+1);		// �ҏW�T�C�Y���Z�b�g
		}else{
			switch( plog->Errsyu ){											
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//				case ERRMDL_EDY:											// �G���[��Edy�̏ꍇ
//					if( plog->Errcod == 42 || plog->Errcod == 43 ){								// 
//						hxtoas2( Asc, (ushort)((plog->ErrBinDat&0xFFFF0000L) >> 16));	// ���2Byte��Ascii�ϊ�
//						hxtoas2( &Asc[4], (ushort)(plog->ErrBinDat&0x0000FFFF) );		// ����2Byte��Ascii�ϊ�
//						sprintf( &csvdata[*csvdata_len], "%s%s", Asc,"," );				// �ҏW�f�[�^�Z�b�g
//						*csvdata_len += (ushort)((strlen( (char*)Asc )+1));				// �ҏW�f�[�^�T�C�Y�Z�b�g
//					}
//					break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				default:													// ���̑�
					sprintf( &csvdata[*csvdata_len], "%lu%s", plog->ErrBinDat,"," );	// �ҏW�f�[�^�Z�b�g
					*csvdata_len += (ushort)(DigitNumCheck( (long)plog->ErrBinDat )+1);		// �ҏW�T�C�Y�Z�b�g
					break;
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�A���[�����O�f�[�^�A�b�v���[�h���̃f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in]		csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�쐬���̊Ǘ��̈�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvArmDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	ulong	wrkl;
	ushort	wrks;
	Arm_log	*plog;	// �A���[���f�[�^�ҏW�|�C���^
	
	Ope_Log_1DataGet( eLOG_ALARM, p->Section, &FTP_buff );	
	plog = (Arm_log *)FTP_buff;

	// ����/�������
	OccurReleaseDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Armdtc, p );

	// �����N�������
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// ���x��
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->Armlev );

	// �⑫���
	if( plog->Arminf ){														// �⑫��񂠂�H
		if( plog->Arminf == 1 && plog->Armsyu != 2){                        // �A�X�L�[�f�[�^�H
			sprintf( &csvdata[*csvdata_len], "%s%s", plog->Armdat,"," );    // �⑫�����Z�b�g
			*csvdata_len += (ushort)(strlen( (char*)plog->Armdat )+1);      // �ҏW�T�C�Y���Z�b�g
		}else{
			switch( plog->Armsyu ){
				case 2:
					if((ALARM_GT_MUKOU_PASS_USE <= plog->Armcod) && (plog->Armcod <= ALARM_GT_N_MINUTE_RULE) || 
						(plog->Armcod == ALARM_GT_MIFARE_READ_CRC_ERR)){
						memcpy(&wrkl,&plog->Armdat[0],4);
						memcpy(&wrks,&plog->Armdat[4],2);
						sprintf( &csvdata[*csvdata_len], "%lu", wrkl );   		// �⑫�����Z�b�g(���ԏꇂ)
						*csvdata_len += (ushort)DigitNumCheck( (long)wrkl );   		// �ҏW�T�C�Y���Z�b�g
						sprintf( &csvdata[*csvdata_len], "%05hu%s", wrks,"," ); // �⑫�����Z�b�g(���ID)
						*csvdata_len += (ushort)(5+1);    						// �ҏW�T�C�Y���Z�b�g(�T��+1)
					}
					else {
						sprintf( &csvdata[*csvdata_len], "%lu%s", plog->ArmBinDat,"," );	// �ҏW�f�[�^�Z�b�g
						*csvdata_len += (ushort)(DigitNumCheck( (long)plog->ArmBinDat )+1);       // �ҏW�T�C�Y�Z�b�g
					}
					break;
				default:
					sprintf( &csvdata[*csvdata_len], "%lu%s", plog->ArmBinDat,"," );	// �ҏW�f�[�^�Z�b�g
					*csvdata_len += (ushort)(DigitNumCheck( (long)plog->ArmBinDat )+1);       // �ҏW�T�C�Y�Z�b�g
					break;
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���j�^���O�f�[�^�A�b�v���[�h���̃f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in]		csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�쐬���̊Ǘ��̈�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvMoniDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{

	ulong	ulinf = 0;
	Mon_log	*plog;	// ���j�^���O�ҏW�|�C���^
	
	Ope_Log_1DataGet( eLOG_MONITOR, p->Section, &FTP_buff );	
	plog = (Mon_log *)FTP_buff;

	// �����N�������
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// ���x��
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->MonLevel );

	// ���j�^���
	if( plog->MonInfo[0] != 0 ){							// Ascii�f�[�^�̏ꍇ
		sprintf( &csvdata[*csvdata_len], "%s%10s", plog->MonInfo,"," );	// ���̂܂ܕ⑫�����Z�b�g
	}else{													// Bin�f�[�^�̏ꍇ
		ulinf = ((ulong)plog->MonInfo[6] << (8 * 3));		// �f�[�^��ҏW
		ulinf += ((ulong)plog->MonInfo[7] << (8 * 2));
		ulinf += ((ulong)plog->MonInfo[8] << (8 * 1));
		ulinf += plog->MonInfo[9];
		if( ulinf ){										// �f�[�^�������
			sprintf( &csvdata[*csvdata_len], "%10lu%s", ulinf,"," );	// �ҏW�����⑫�����Z�b�g
		}
	}
	
	if( plog->MonInfo[0] != 0 || ulinf != 0 ){				// �ҏW����f�[�^������ꍇ
		*csvdata_len += (ushort)(sizeof( plog->MonInfo )+1);// �ҏW�T�C�Y�����Z
	}
	

}

//[]----------------------------------------------------------------------[]
///	@brief			���샂�j�^���O�f�[�^�A�b�v���[�h���̃f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in]		csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�쐬���̊Ǘ��̈�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvOpeMoniDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{
	Ope_log	*plog;	// ���샂�j�^���O�ҏW�|�C���^
	
	Ope_Log_1DataGet( eLOG_OPERATE, p->Section, &FTP_buff );	
	plog = (Ope_log *)FTP_buff;

	// �����N�������
	TimeDataEdit( &csvdata[*csvdata_len], csvdata_len, &plog->Date_Time );
	
	// ���x��
	LevelDataEdit( &csvdata[*csvdata_len], csvdata_len, plog->OpeLevel );

	// �ύX�O�f�[�^
	if( plog->OpeBefore ){
		sprintf( &csvdata[*csvdata_len], "%lu%s", plog->OpeBefore,"," );
		*csvdata_len += (ushort)(DigitNumCheck((long)plog->OpeBefore)+1);
	}else{
		csvdata[*csvdata_len] = ',';
		(*csvdata_len)++;
	}

	// �ύX��f�[�^
	if( plog->OpeAfter ){
		sprintf( &csvdata[*csvdata_len], "%lu%s", plog->OpeAfter,"," );
		*csvdata_len += (ushort)(DigitNumCheck((long)plog->OpeAfter)+1);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�Ԏ����f�[�^�A�b�v���[�h���̃f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		csvdata		: �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^
///	@param[in]		csvdata_len	: �ϊ���f�[�^�̃T�C�Y���Z�b�g����G���A�ւ̃|�C���^
///	@param[in]		p			: �f�[�^�쐬���̊Ǘ��̈�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/06/18
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	AppServ_ConvCarInfoDataCreate( char *csvdata, ulong *csvdata_len, t_AppServ_Bin_toCsv_Param *p )
{

	flp_com	*pdata = &FLAPDT.flp_data[p->Section];	
	uchar	i;
	
	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->mode,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->mode ) + 1); 
	
	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->nstat.word,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->nstat.word ) + 1);
	
	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->ostat.word,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->ostat.word ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->ryo_syu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->ryo_syu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->year ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->mont,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->date,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->date ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->hour,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->hour ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->minu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->minu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->passwd,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->passwd ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->uketuke,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->uketuke ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->u_year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->u_year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->u_mont,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->u_hour,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_hour ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->u_minu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->u_minu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->bk_syu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_syu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->s_year,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_year ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_mont,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_mont ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_date,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_date ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_hour,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_hour ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->s_minu,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->s_minu ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->lag_to_in.BYTE,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->lag_to_in.BYTE ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->issue_cnt,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->issue_cnt ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->bk_wmai,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_wmai ) + 1);

	sprintf( &csvdata[*csvdata_len], "%lu%s",pdata->bk_wari,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_wari ) + 1);

	sprintf( &csvdata[*csvdata_len], "%lu%s",pdata->bk_time,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_time ) + 1);

	sprintf( &csvdata[*csvdata_len], "%u%s",pdata->bk_pst,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->bk_pst ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->in_chk_cnt,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->in_chk_cnt ) + 1);

	sprintf( &csvdata[*csvdata_len], "%ld%s",pdata->timer,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->timer ) + 1);

	sprintf( &csvdata[*csvdata_len], "%d%s",pdata->car_fail,"," );
	*csvdata_len += ( DigitNumCheck( (long)pdata->car_fail ) + 1);

	for( i=0; i<(TBL_CNT(pdata->Dummy)); i++ ){
		sprintf( &csvdata[*csvdata_len], "%lu%s",pdata->Dummy[i],"," );
		*csvdata_len += ( DigitNumCheck( (long)pdata->Dummy[0] ) + 1);
	}

}

//[]----------------------------------------------------------------------[]
///	@brief			����/�����f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: �����݃f�[�^
///	@return			count		: �����݌���
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	OccurReleaseDataEdit( char *data, ulong *len, uchar dtc, t_AppServ_Bin_toCsv_Param *p )
{
	uchar	i = 0;
	
	switch( dtc ){
		case	0:
			i = 7;
			break;
		case	1:
			i = 6;
			break;
		default:
			i = 2;
			break;
	}
	sprintf( data, "%s%s",csv_edit[p->Kind].title_data[i],"," );
	*len += (ushort)(strlen(csv_edit[p->Kind].title_data[i])+1);
}

//[]----------------------------------------------------------------------[]
///	@brief			�����N�����f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: �����݃f�[�^
///	@return			count		: �����݌���
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	TimeDataEdit( char *data, ulong *len, date_time_rec *time )
{
	sprintf( data, "%04d/%02d/%02d %02d:%02d%s",
				time->Year,
				time->Mon,
				time->Day,
				time->Hour,
				time->Min,
				"," );

	*len += 16+1;
}

//[]----------------------------------------------------------------------[]
///	@brief			���x���f�[�^�쐬
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: �����݃f�[�^
///	@return			count		: �����݌���
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	LevelDataEdit( char *data, ulong *len, uchar level )
{
	sprintf( data, "%d%s", level,"," );
	*len += 2;
}

//[]----------------------------------------------------------------------[]
///	@brief			�f�[�^�����݌����`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: �����݃f�[�^
///	@return			count		: �����݌���
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
uchar	DigitNumCheck( long data )
{
	uchar count = 0;
	
	switch( data ){
		case	0:
			count = 1;
			break;
		case   -1:
			count = 2;
			break;
		default:
			while( data != 0 ){
				data /= 10;
				count++;
			}
			break;
	}
	return count;
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�֘A�t�@�C���T�C�Y�擾(FTP�p)
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: �����݃f�[�^
///	@return			count		: �����݌���
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2009/01/22
///					Update	: 
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
ulong	DataSizeCheck( char *data, uchar kind )
{
	ulong Total;
	t_AppServ_Bin_toCsv_Param *p = &AppServ_BinCsvSave.Bin_toCsv_Param;

	Total = 0;
	AppServ_ConvErrArmCSV_Init( kind );
	switch( p->Kind ){	// ��Ǝ��
		case 0:			// �G���[���O
			Total = (p->datasize * sizeof( Err_log ));	
			break;
		case 1:			// �A���[�����O
			Total = (p->datasize * sizeof( Arm_log ));	
			break;
		case 2:			// ���j�^���O
			Total = (p->datasize * sizeof( Mon_log ));	
			break;
		case 3:			// ���샂�j�^���O
			Total = (p->datasize * sizeof( Ope_log ));	
			break;
		default:
			break;
	}

	return Total;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvCSVtoFLAPDT_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��f�[�^�A�b�v���[�h�������֐��icsv�@���@bin�j
 *| param	: lock_info_image	�ϊ���Ԏ��f�[�^�i�[�惏�[�N�G���A
 *| param	: image_size		���[�N�G���A�T�C�Y
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSVtoFLAPDT_Init( char *flapdt_image, ulong image_size )
{

	if (image_size < sizeof(LockInfo)) {
		return	(uchar)0;
	}

	AppServ_CtoB_FLAPDT.flapdt_image = (flp_rec*)flapdt_image;
	AppServ_CtoB_FLAPDT.image_size = image_size;
	AppServ_CtoB_FLAPDT.CallCount = 0;
	memset(flapdt_image, 0, (size_t)image_size);

	return	(uchar)1;
}

#define	Read_Col(a,b,c)	AppServ_CnvCsvFLAPDT_1ColumnRead(a,b,c)
/*[]----------------------------------------------------------------------[]
 *|	name	: AppServ_CnvCSVtoFLAPDT()
 *[]----------------------------------------------------------------------[]
 *| summary	: 
 *| param	: csvdata		�ϊ����f�[�^
 *| param	: csvdata_len	�ϊ����f�[�^�T�C�Y
 *| param	: finish		�ϊ��I���̃t���O�i��������R�Q�S��j
 *| return	: 1:OK	0:NG
 *[]----------------------------------------------------------------------[]*/
uchar	AppServ_CnvCSVtoFLAPDT(const char *csvdata, ushort csvdata_len, ulong *finish )
{
	uchar	ret;
	long	BinData;
	const char	*pNextCsvData;
	flp_com	*p = &AppServ_CtoB_FLAPDT.flapdt_image->flp_data[0];
	uchar	i;
	
	if (AppServ_CtoB_FLAPDT.CallCount >= ITEM_MAX_LOCK_INFO) {
		if (csvdata[0] == '\r') {
			*finish = 1;
			return 1;
		}
		else {
			return 0;	/* �ŏI�s�ȍ~�Ƀf�[�^������ */
		}
	}
	
	pNextCsvData = csvdata;

	// �R�����g�s�͔�΂��āA�f�[�^�s���擾����B
	while(1){
		ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );

		// �G���[
		if( ret & BIT_ERROR )
			return	0;
			
		if( 0 == (ret & BIT_NEXT_EXIST) )						/* ���f�[�^�Ȃ�(Y) */
			return 1;		// ��M�p��

		if( ret & BIT_COMMENT )
			continue;

		if( ret & BIT_DATA_EXIST )
			break;


	}
	// �}�C�i�X�̒l�������̂ŁA�ް��͈̔̓`�F�b�N�͂��Ȃ��B
	// mode
	if (ret == 0x03) {
		p->mode = (ushort)BinData;
	} else {
		return 0;
	}

	// nstat
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->nstat.word = (ushort)BinData;
	} else {
		return 0;
	}

	// ostat
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->ostat.word = (ushort)BinData;
	} else {
		return 0;
	}

	// ryo_syu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->ryo_syu = (ushort)BinData;
	} else {
		return 0;
	}

	// year
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->year = (ushort)BinData;
	} else {
		return 0;
	}

	// mont
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->mont = (uchar)BinData;
	} else {
		return 0;
	}

	// date
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->date = (uchar)BinData;
	} else {
		return 0;
	}

	// hour
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->hour = (uchar)BinData;
	} else {
		return 0;
	}

	// minu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->minu = (uchar)BinData;
	} else {
		return 0;
	}

	// passwd
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->passwd = (ushort)BinData;
	} else {
		return 0;
	}

	// uketuke
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->uketuke = (ushort)BinData;
	} else {
		return 0;
	}

	// u_year
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_year = (ushort)BinData;
	} else {
		return 0;
	}

	// u_mont
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_mont = (uchar)BinData;
	} else {
		return 0;
	}

	// u_date
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_date = (uchar)BinData;
	} else {
		return 0;
	}

	// u_hour
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_hour = (uchar)BinData;
	} else {
		return 0;
	}

	// u_minu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->u_minu = (uchar)BinData;
	} else {
		return 0;
	}

	// bk_syu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_syu = (ushort)BinData;
	} else {
		return 0;
	}

	// s_year
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_year = (ushort)BinData;
	} else {
		return 0;
	}

	// s_mont
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_mont = (uchar)BinData;
	} else {
		return 0;
	}

	// s_date
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_date = (uchar)BinData;
	} else {
		return 0;
	}

	// s_hour
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_hour = (uchar)BinData;
	} else {
		return 0;
	}

	// s_minu
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->s_minu = (uchar)BinData;
	} else {
		return 0;
	}

	// lag_to_in
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->lag_to_in.BYTE = (uchar)BinData;
	} else {
		return 0;
	}

	// issue_cnt
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->issue_cnt = (uchar)BinData;
	} else {
		return 0;
	}

	// bk_wmai
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_wmai = (ushort)BinData;
	} else {
		return 0;
	}

	// bk_wari
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_wari = (ulong)BinData;
	} else {
		return 0;
	}

	// bk_time
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_time = (ulong)BinData;
	} else {
		return 0;
	}

	// bk_pst
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->bk_pst = (ushort)BinData;
	} else {
		return 0;
	}

	// in_chk_cnt
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->in_chk_cnt = (short)BinData;
	} else {
		return 0;
	}

	// timer
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->timer = (long)BinData;
	} else {
		return 0;
	}

	// car_fail
	ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
	if (ret == 0x03) {
		p->car_fail = (uchar)BinData;
	} else {
		return 0;
	}

	// Dummy
	for( i=0; i<(TBL_CNT(p->Dummy));i++ ){
		ret = Read_Col( pNextCsvData, &BinData, &pNextCsvData );
		if (ret == 0x03) {
			p->Dummy[i] = (ulong)BinData;
		} else {
			if( ret & BIT_DATA_EXIST ){
				p->Dummy[i] = (ulong)BinData;
			}else if( i != (TBL_CNT(p->Dummy)-1) ){
				return 0;
			}else{
				return 0;
			}
		}
	}

	// 1Line�ǂݍ��݌�Ƃ�RAM�̃f�[�^�ɔ��f������B
	// �X�V���̒�d�ɂ͍ēx�_�E�����[�h�����{���邱�ƂőΉ�����
	memcpy(&FLAPDT.flp_data[AppServ_CtoB_FLAPDT.CallCount], p, sizeof(flp_com));		// RAM�ɔ��f
	
	// �Ăяo���񐔃C���N�������g
	AppServ_CtoB_FLAPDT.CallCount++;

	// �I���t���O�ݒ�
	if (AppServ_CtoB_FLAPDT.CallCount >= ITEM_MAX_LOCK_INFO) {
		*finish = 1;
		wopelg2(OPLOG_CARINFO_DL, 0, 0);	// �Ԏ����_�E�����[�h���{
	} else {
		*finish = 0;
	}

	return 1;

}

/*[]-----------------------------------------------------------------------[]*/
/*|	�Ԏ����_�E�����[�h �T�u���[�`��										|*/
/*|																			|*/
/*|	CSV�`���f�[�^�̂P�J������HEX�f�[�^�ɕϊ��B								|*/
/*|	","(�J���})�ŋ�؂�ꂽ�f�[�^�P�̉�͂ƕϊ����s���B					|*/
/*|	�f�[�^���Z�N�V�����ԍ��������R�����g�s�ł���ꍇ�́A���̏���Ԃ��B	|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = PC�����M����CSV�`���f�[�^					|*/
/*|				   pBinData = HEX�ϊ��l�Z�b�g�G���A�ւ̃|�C���^				|*/
/*|							  return�l=1���̂ݗL��							|*/
/*|				   pNextCsvData = ���f�[�^�̐擪�A�h���X					|*/
/*| RETURN VALUE : 0=�f�[�^�Ȃ��A1=���l�f�[�^�L��ipBinData�ɃZ�b�g�j		|*/
/*|				   2=�R�����g�s�i�ϊ��Ȃ��j                                 |*/
/*|				   b0=1:BinData����i�ϊ����{�j								|*/
/*|				   b1=1:���f�[�^����A0:�ŏI�f�[�^							|*/
/*|				   b2=1:�R�����g�J����(�擪)�A0:�R�����g�łȂ�				|*/
/*|				   b7=1:�f�[�^�G���[����									|*/
/*|			�i��j															|*/
/*|				   06h=�R�����g�J�����̂��߃f�[�^�Ȃ�  ���f�[�^����			|*/
/*|				   03h=���l�f�[�^�L��(pBinData�ɃZ�b�g)���f�[�^����			|*/
/*|				   01h=���l�f�[�^�L��(pBinData�ɃZ�b�g)���f�[�^�Ȃ�(�ŏI)	|*/
/*|				   04h=�R�����g�J�����̂��߃f�[�^�Ȃ�  ���f�[�^�Ȃ�(�ŏI)	|*/
/*|				   80h=�`���G���[�i�ϊ��Ȃ��j								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Namioka													|*/
/*| Date         : 2009/06/18												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
uchar	AppServ_CnvCsvFLAPDT_1ColumnRead( const char *CsvData, long *pBinData, const char **pNextCsvData )
{
	char	Asc[15];
	ushort	i,j;											/* i= read index */
															/* j= data get count */
	char	TgtChar;
	uchar	ret=0;
long	BinData;

	ret = 0;

	for( i=0, j=0; j<15; ++i ){								/* �ő�15�����擾����܂� */
		TgtChar = CsvData[i];
		if( ' ' == TgtChar ){								/* �X�y�[�X�͓ǂݔ�΂� */
			continue;
		}

		if( ',' == TgtChar ){								/* ��؂蕶��(������) */
			*pNextCsvData = (char*)&CsvData[i+1];					/* ���A�h���X�Z�b�g */
			ret |= BIT_NEXT_EXIST;
			goto AppServ_CnvCsvParam_1ColumnRead_10;		/* 1�J�������o���� ������ */
		}

		else if( (0x0d == TgtChar) ||						/* �s�����R�[�h */
				 (0x0a == TgtChar) ||
				 (0x1a == TgtChar) ){

AppServ_CnvCsvParam_1ColumnRead_10:
			/** 1�J�������o���� ���� **/
			if( 0 == j ){									/* �f�[�^�Ȃ�(Y) */
				//break;
			}
			else if( '[' == Asc[0] ){						/* �R�����g�s(Y) */
				ret |= BIT_COMMENT;
				//break;
			}
			else{
				if( 1 == DecAsc_to_LongBin_Minus((uchar*)&Asc[0], (uchar)j, &BinData) ){
															/* Ascii -> Hex�ϊ� */
					*pBinData = BinData;					/* Hex data set */
					ret |= BIT_DATA_EXIST;					/* �f�[�^���� */
				}
				else{										/* ���f�[�^�ُ� */
					ret |= BIT_ERROR;
				}
				//break;
			}
			break;
		}

		else{												/* �I���R�[�h�ł͂Ȃ� */
			Asc[j++] = TgtChar;
			if( 15 < j ){									/* �f�[�^�傫����error */
				ret |= BIT_ERROR;
				break;
			}
		}
	}
	return	ret;
}


/*[]-----------------------------------------------------------------------[]*/
/*|	Text���M�f�[�^�t�H�[�}�b�g�쐬											|*/
/*|																			|*/
/*|	PC����_�E�����[�h���ꂽ�f�[�^��FlashROM�i�[�`���ɕϊ�����B			|*/
/*|	�����f�[�^��ASCII or Shift-JIS�ɑΉ��BUnicode�͕s�B					|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : formated = �ϊ���f�[�^�i�[�G���A�ւ̃|�C���^			|*/
/*|				   text = �ϊ����f�[�^(PC�����M�����f�[�^)�ւ̃|�C���^	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Namioka													|*/
/*| Date         : 2009/08/28												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	AppServ_FormatTextData( char formated[][36], const char *text, ushort text_len, uchar line_num )
{
	ushort	Line = line_num;
	ushort	Col;
	ushort	i;

	memset( &formated[0][0], ' ', (size_t)(Line*36) );			/* set area space clear */

	Line = 0;
	Col  = 0;
	for( i=0; i<text_len; ++i ){
		if( 0x0d == text[i] ){
			++Line;
			/* �s��Max�`�F�b�N */
			if( line_num <= Line ){
				return;
			}
			Col = 0;
			if( 0x0a == text[i+1] ){
				++i;
			}
			continue;
		}
		else{
			/* ������Max�`�F�b�N */
			if( 36 <= Col ){
				continue;
			}
			formated[Line][Col++] = text[i];
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	Text���M�f�[�^�t�H�[�}�b�g�쐬											|*/
/*|																			|*/
/*|	���Z�@���f�[�^�ɉ��s�R�[�h(CRLF)��}������								|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : inserted = ���s�}����f�[�^�i�[�G���A�ւ̃|�C���^		|*/
/*|				   text = ���s�}�����f�[�^(���Z�@���f�[�^)�ւ̃|�C���^		|*/
/*| RETURN VALUE : �ϊ���̃f�[�^��										    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Namioka													|*/
/*| Date         : 2009/08/28												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	AppServ_InsertCrlfToTextData( char *inserted, const char text[][36], uchar line_num )
{
	ushort	i, len;
	
	len = 0;
	for (i = 0; i < line_num; i++) {
		/* copy 1 line */
		memcpy(&inserted[len], &text[i][0], 36);
		len += 36;
		/* insert cr & lf */
		inserted[len++] = '\r';
		inserted[len++] = '\n';
	}
	
	return len;
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�f�[�^�A�b�v���[�h�������i�o�C�i���`���Ή��j
//[]----------------------------------------------------------------------[]
///	@param[in]		kind ���O���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2012/05/04
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	AppServ_SaleTotal_LOG_Init( uchar kind, ulong size )
{

	// �{���ABin_toCsv_Param�͋��ʃp�����[�^�̃A�b�v���[�h�p�̈悾���A���O�n�̃A�b�v���[�h�ɂ�
	// �K�v�ȃG���A�̂ݎg�p���ĕ��p����B

	/* ��ƃG���A�N���A */
	memset( &AppServ_BinCsvSave.Bin_toCsv_Param, 0, sizeof( t_AppServ_Bin_toCsv_Param ));
	AppServ_BinCsvSave.Bin_toCsv_Param.Kind = kind;			// ��Ǝ�ʂ̕ێ�
	AppServ_BinCsvSave.Bin_toCsv_Param.Item = LogDatMax[kind][0];			// ��Ǝ�ʂ̕ێ�

	AppServ_BinCsvSave.Bin_toCsv_Param.datasize = (size/LogDatMax[kind][0]);

}

//[]----------------------------------------------------------------------[]
///	@brief			���O�f�[�^�A�b�v���[�h�������i�o�C�i���`���Ή��j
//[]----------------------------------------------------------------------[]
///	@param[in]		kind ���O���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Namioka
///	@date			Create	: 2012/05/04
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	AppServ_SaleTotal_LOG_edit( char *buf, ulong *len, uchar *finish )
{

	t_AppServ_Bin_toCsv_Param	*p = &AppServ_BinCsvSave.Bin_toCsv_Param;
	
	Ope_Log_1DataGet( p->Kind, p->Section, &FTP_buff );

	p->Section++;
	memcpy( buf, FTP_buff, p->Item );
	*len = p->Item;
	
	if( p->Section >= p->datasize ){
		*finish = 1;
	}
}

void Get_Result_dat(uchar no, uchar cnt, uchar *dat)
{
	
	switch(no){
	case 0:		// �Ј��ԍ�
	case 1:		// ��������
	case 2:		// �p�^�[���ԍ�
		break;
	case 3:
		memcpy(dat , &Chk_result.set[0], cnt);
		break;
	case 4:
		memcpy(dat , &Chk_result.moj[0], cnt);
		break;
	case 5:
		memcpy(dat , &Chk_result.key_disp[0], cnt);
		break;
	case 6:
		memcpy(dat , &Chk_result.mag[0], cnt);
		break;
	case 7:
		memcpy(dat , &Chk_result.led_shut[0], cnt);
		break;
	case 8:
		memcpy(dat , &Chk_result.sw[0], cnt);
		break;
	case 9:
		memcpy(dat , &Chk_result.r_print[0], cnt);
		break;
	case 10:
		memcpy(dat , &Chk_result.j_print[0], cnt);
		break;
	case 11:
		memcpy(dat , &Chk_result.ann[0], cnt);
		break;
	case 12:
		memcpy(dat , &Chk_result.sig[0], cnt);
		break;
	case 13:
		memcpy(dat , &Chk_result.coin[0], cnt);
		break;
	case 14:
		memcpy(dat , &Chk_result.note[0], cnt);
		break;
	default:
		break;
	}
}
const	ushort	AppServ_ChkResult_NumTbl[CHK_RESULT_NUMMAX]={
		0,
		CHK_RESULT01,									/* ��������01�ݒ萔			*/
		CHK_RESULT02,									/* ��������02�ݒ萔			*/
		CHK_RESULT03,									/* ��������03�ݒ萔			*/
		CHK_RESULT04,									/* ��������04�ݒ萔			*/
		CHK_RESULT05,									/* ��������05�ݒ萔			*/
		CHK_RESULT06,									/* ��������06�ݒ萔			*/
		CHK_RESULT07,									/* ��������07�ݒ萔			*/
		CHK_RESULT08,									/* ��������08�ݒ萔			*/
		CHK_RESULT09,									/* ��������09�ݒ萔			*/
		CHK_RESULT10,									/* ��������10�ݒ萔			*/
		CHK_RESULT11,									/* ��������11�ݒ萔			*/
		CHK_RESULT12,									/* ��������12�ݒ萔			*/
	
};
char	*Chk_res_p[CHK_RESULT_NUMMAX];				/* ���Ұ������ð���				*/
/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��i�������j				|*/
/*|																			|*/
/*|	RAM���f�[�^��CSV�`���l�ɕϊ�����B���̂��߂̑O�����B					|*/
/*|	findfirst, findnext�̂悤�ɍŏ��ɖ{�֐��A�ȍ~��AppServ_CnvParam2CSV()��	|*/
/*|	�����Ǐo�����s���B														|*/
/*|	���� datasize�ɂ�1112byte�ȏ��p�ӂ��邱�ƁB							|*/
/*|		 AppServ_CnvParam2CSV()��CSV��s���̃f�[�^���i100�A�C�e�����j��		|*/
/*|		 �Z�b�g���邽�߁B													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : param_info = ���ʁE�ʃp�����[�^�f�[�^�̏��e�[�u��	|*/
/*|								�ւ̃|�C���^								|*/
/*|				   datasize = AppServ_CnvParam2CSV()�ň��ɃZ�b�g�\��	|*/
/*|							  �f�[�^�T�C�Y�B�Ăь����W���[���̃o�b�t�@�T�C�Y|*/
/*| RETURN VALUE : 1 = OK                                                   |*/
/*|				   0 = NG (���[�N�G���A�T�C�Y��������)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChkResultCSV_Init( void )
{

	/* ��ƃG���A�N���A */
	memset( &AppServ_BinCsvSave.Bin_toCsv_Param, 0, sizeof( t_AppServ_Bin_toCsv_Param ));
	
	Chk_res_p[0]	= (char *)&Chk_res_ftp.Chk_Res00;											/*								*/
	Chk_res_p[1]	= (char *)&Chk_res_ftp.Chk_Res01;											/*								*/
	Chk_res_p[2]	= (char *)&Chk_res_ftp.Chk_Res02;											/*								*/
	Chk_res_p[3]	= (char *)&Chk_res_ftp.Chk_Res03;											/*								*/
	Chk_res_p[4]	= (char *)&Chk_res_ftp.Chk_Res04;											/*								*/
	Chk_res_p[5]	= (char *)&Chk_res_ftp.Chk_Res05;											/*								*/
	Chk_res_p[6]	= (char *)&Chk_res_ftp.Chk_Res06;											/*								*/
	Chk_res_p[7]	= (char *)&Chk_res_ftp.Chk_Res07;											/*								*/
	Chk_res_p[8]	= (char *)&Chk_res_ftp.Chk_Res08;											/*								*/
	Chk_res_p[9]	= (char *)&Chk_res_ftp.Chk_Res09;											/*								*/
	Chk_res_p[10]	= (char *)&Chk_res_ftp.Chk_Res10;											/*								*/
	Chk_res_p[11]	= (char *)&Chk_res_ftp.Chk_Res11;											/*								*/
	Chk_res_p[12]	= (char *)&Chk_res_ftp.Chk_Res12;											/*								*/

}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��i�Ǐo���j				|*/
/*|																			|*/
/*|	RAM���f�[�^��CSV�`���l�ɕϊ����A�o�b�t�@�ɃZ�b�g����B					|*/
/*|	�f�[�^��100�� ���� 1�Z�N�V�������̏��������ƂȂ�B						|*/
/*|	�{�֐�Call�O�� AppServ_ConvParam2CSV_Init() ��Call���邱�ƁB			|*/
/*|	�ŏ�����ǂݏo���Ȃ����ꍇ���A�������֐���Call���邱�ƁB				|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*|				   finish = �S�Z�N�V�����̕ϊ������� 1���Z�b�g�B�ȊO��0�B	|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChkResultCSV( char *csvdata, ulong *csvdata_len, uchar *finish )
{
	ulong	TotalLen;
	uchar	ret;

	*finish = 0;

	if( 0 == AppServ_BinCsvSave.Bin_toCsv_Param.Phase ){	/* �擪�s�i���o���j���M�t�F�[�Y (Y)	*/
		AppServ_ConvChk_ResultCSV_Sub1( csvdata, csvdata_len );	/* ���o���쐬 */
		AppServ_BinCsvSave.Bin_toCsv_Param.Phase = 1;		/* ���t�F�[�Y�̓f�[�^���ϊ� */
		AppServ_BinCsvSave.Bin_toCsv_Param.Section = 1;
		return;
	}

	/** �f�[�^���ϊ��t�F�[�Y **/

	TotalLen = 0;

	AppServ_ConvChk_ResultCSV_Sub2( csvdata, &TotalLen, 			/* �擪�J�������o���쐬 */
									AppServ_BinCsvSave.Bin_toCsv_Param.Section,
									(ulong)1 );

	/* 1�Z�N�V�����Z�b�g���� ���� �����݃o�b�t�@�T�C�YLimit�܂Ńf�[�^�Z�b�g */
	ret = AppServ_ConvChk_ResultCSV_Sub3( csvdata, &TotalLen );
	*csvdata_len = TotalLen;
	if( ret == 1 ){											/* �I�� (Y) */
		*finish = 1;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub1�j					|*/
/*|																			|*/
/*|	CSV�t�@�C���̂P�s�ڌ��o�����쐬����B									|*/
/*|	�T�C�Y��493byte�ƂȂ邪�A�������ݗp�o�b�t�@��512byte�ȏ�ł���̂�		|*/
/*|	��C�Ɉ�s�����Z�b�g����B												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChk_ResultCSV_Sub1( char *csvdata, ulong *csvdata_len )
{
	ushort	i;
	ushort	SetLen;
	ushort	TotalLen;
	uchar	Asc[8];

	TotalLen = 0;
	Asc[0] = ',';
	Asc[1] = '[';

	for( i=0; i<20; ++i ){

		cnvdec4( (char*)&Asc[2], (long)i );						/* Bin to Ascii�ϊ� */

		SetLen = 0;
		if( 10 > i ){										/* �����P���� (Y) */
			Asc[2] = Asc[5];
			Asc[3] = ']';
			SetLen = 4;
		}
		else{												/* �����Q���� */
			Asc[2] = Asc[4];
			Asc[3] = Asc[5];
			Asc[4] = ']';
			SetLen = 5;
		}

		memcpy( (void*)&csvdata[TotalLen], (const void*)&Asc[0], (size_t)SetLen );
		TotalLen += SetLen;
	}

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;

	*csvdata_len = TotalLen + 2;
	return;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub2�j					|*/
/*|																			|*/
/*|	�e�s�̐擪�i�P�J�����ځj���o���쐬����									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*|				   ParamKind = 0:���ʁC 1:��								|*/
/*|				   SectionNum = �Z�N�V�����ԍ��i1�`99�j						|*/
/*|				   ItemNum = �擪��Item�ԍ��i1�`9999�j						|*/
/*| RETURN VALUE : void                                                     |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	AppServ_ConvChk_ResultCSV_Sub2( char *csvdata, ulong *csvdata_len,
								   ushort SectionNum, ushort ItemNum )
{
	csvdata[0] = '[';

	cnvdec2( &csvdata[1], (short)SectionNum );
	csvdata[3] = '-';
	cnvdec4( &csvdata[4], (long)ItemNum );
	csvdata[8] = ']';
	csvdata[9] = ',';
	*csvdata_len = 10;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���ʁ^�ʃp�����[�^�A�b�v���[�h���̃f�[�^�ϊ��iSub3�j					|*/
/*|																			|*/
/*|	�f�[�^�Z�b�g�iCSV�쐬�j����												|*/
/*| 1�s���̃f�[�^���Z�b�g�B�i�P�Z�N�V�����I�� ���� 100�A�C�e���j			|*/
/*|	�����ɂ�CR,LF���Z�b�g����B(Max1112byte�Z�b�g����)						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : csvdata = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^			|*/
/*|				   csvdata_len = �ϊ���f�[�^�̃T�C�Y���Z�b�g����			|*/
/*|								 �G���A�ւ̃|�C���^							|*/
/*|				   ��ƃG���A�iAppServ_BinCsvSave�j�̏����g���B			|*/
/*				   �X�V���s���B												|*/
/*| RETURN VALUE : 1=1�Z�N�V�����I���B 0=�����݃o�b�t�@Limit�ɂ��I��      |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda													|*/
/*| Date         : 2005/06/24												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	AppServ_ConvChk_ResultCSV_Sub3( char *csvdata, ulong *csvdata_len )
{
	char	i;
	ushort	TgtSection;		/* target Section number (0- ) */
	ushort	ItemCount;		/* total Item count in target Section */
	

	ushort	TotalLen;				/* �������񂾃T�C�Y */

	TgtSection	= AppServ_BinCsvSave.Bin_toCsv_Param.Section;
	ItemCount = AppServ_ChkResult_NumTbl[TgtSection];

	TotalLen = *csvdata_len;
	/*  */
	switch(TgtSection){
		case 1:
			/* �������� */
			sprintf( &csvdata[TotalLen], "%02d%02d%02d%02d%02d%s",
					Chk_res_ftp.Chk_Res01.Chk_date.Year%100,
					Chk_res_ftp.Chk_Res01.Chk_date.Mon,
					Chk_res_ftp.Chk_Res01.Chk_date.Day,
					Chk_res_ftp.Chk_Res01.Chk_date.Hour,
					Chk_res_ftp.Chk_Res01.Chk_date.Min,
					"," );
			TotalLen += 10+1;
			/* �Ј��ԍ� */
			sprintf( &csvdata[TotalLen], "%04d%s", Chk_res_ftp.Chk_Res01.Kakari_no,",");
			TotalLen += 5;
			/* ���i���@ */
			memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res01.Machine_No, 6 );
			TotalLen += 6;
			csvdata[TotalLen] = ',';
			TotalLen++;
			/* ���f�� */
			memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res01.Model, 8 );
			TotalLen += 8;
			csvdata[TotalLen] = ',';
			TotalLen++;
			/* �V�X�e���\�� */
			sprintf( &csvdata[TotalLen], "%02d%s", Chk_res_ftp.Chk_Res01.System,",");
			TotalLen += 3;
			/* �T�u�`���[�u���� */
			sprintf( &csvdata[TotalLen], "%03d%s", Chk_res_ftp.Chk_Res01.Sub_Money,",");
			TotalLen += 4;
			break;
		case 2:						/* �\�t�g�o�[�W���� */
			for(i=0;i<3;i++){
				memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res02.Version[i], 8 );
				TotalLen+=8;
				csvdata[TotalLen] = ',';
				TotalLen++;
			}
			break;
		case 5:						/* ���W���[���� */
			for(i=0;i<27;i++){
				memcpy( &csvdata[TotalLen], Chk_res_ftp.Chk_Res05.Mojule[i], 8 );
				TotalLen+=8;
				csvdata[TotalLen] = ',';
				TotalLen++;
			}
			break;
		case 3:						/* ���C���[�_�[�f�B�b�v�X�C�b�`��� */
			for(i = 0; i<3; i++){
				sprintf( &csvdata[TotalLen], "%01d%01d%01d%01d%s", 
					Chk_res_ftp.Chk_Res03.Dip_sw[i][0],
					Chk_res_ftp.Chk_Res03.Dip_sw[i][1],
					Chk_res_ftp.Chk_Res03.Dip_sw[i][2],
					Chk_res_ftp.Chk_Res03.Dip_sw[i][3],
					","
				);
				TotalLen += 5;
			}
			sprintf( &csvdata[TotalLen], "%01d%s", 
				Chk_res_ftp.Chk_Res03.Dip_sw6,
				","
			);
			TotalLen += 2;
			break;
		default:
			/* ���ʂ� 0 or 1 �̍���*/
			for(i=0;i<ItemCount;i++){
				sprintf( &csvdata[TotalLen], "%01d%s", Chk_res_p[TgtSection][i],",");
				TotalLen += 2;
			}
		break;
	}
	

	csvdata[TotalLen] = 0x0d;
	csvdata[TotalLen+1] = 0x0a;
	TotalLen += 2;

	*csvdata_len = TotalLen;

	/* �I���v������ */
	TgtSection++;

	if(TgtSection >= CHK_RESULT_NUMMAX){
		return 1;
	}
	else{
		AppServ_BinCsvSave.Bin_toCsv_Param.Section = TgtSection;			/* ���̃Z�N�V������ */
		AppServ_BinCsvSave.Bin_toCsv_Param.Item	= 0;
	}
	return	0;
}

/*[]-----------------------------------------------------------------------[]*/
/*|	�T�C�Y�ύX���ꂽ�o�b�N�A�b�v�f�[�^�̃��X�g�A����						|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	PARAMETER	 : area_tbl = �o�b�N�A�b�v�f�[�^���(�v���O����)			|*/
/*|				   area_tbl_wk = �o�b�N�A�b�v�f�[�^���(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_BackupData(const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ushort	area, area_num;

	// �G���A���̌���(���Ȃ��ق��ɍ��킹��)
	area_num = area_tbl->area_num;
	if (area_num > area_tbl_wk->area_num) {
		area_num = area_tbl_wk->area_num;
	}

	for (area = 0; area < area_num; area++) {
		// ���Ɂi���Ԉʒu�j����RSW=8�N�����Ƀ��X�g�A����邽�߂����ł̓��X�g�A���Ȃ�
		if( area == OPE_DTNUM_FLAPDT ){
			continue;
		}

		// �A�h���X�������ꍇ�͊i�[�ΏۊO
		if(area_tbl_wk->from_area[area].from_address == 0L){
			continue;
		}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
		if(area == OPE_DTNUM_SKY || area == OPE_DTNUM_SKYBK) {
			// ���O�o�[�W�����Ɋւ�炸�W�v���O�͂����Ń��X�g�A
			AppServ_Restore_SYUKEI(area, area_tbl, area_tbl_wk);
			continue;
		} else if(area == OPE_DTNUM_LOG_DCSYUUKEI) {
			// ���O�o�[�W�����Ɋւ�炸�d�q�}�l�[��ʖ��W�v�͂����Ń��X�g�A
			AppServ_Restore_SYUKEISP(area, area_tbl, area_tbl_wk);
		}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810105 MH321800(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
		else if(area == OPE_DTNUM_EC_ALARM_TRADE_LOG) {
			//  ���O�o�[�W�����Ɋւ�炸��������������O�͂����Ń��X�g�A
			AppServ_Restore_ECALAMLOG(area, area_tbl_wk);
			continue;
		}
// MH810105 MH321800(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
		// �o�b�N�A�b�v�f�[�^�̍\���̃T�C�Y���ύX���ꂽ�ꍇ���X�g�A������ǉ����邱��
		// (AppServ_Restore_ToV04�֐����ƍ����ւ���C���[�W)
		// ���O�ƃo�b�N�A�b�v�f�[�^�ŋ��ʂɎg�p���Ă���\���͉̂��L��4��
		// �W�v���O(SYUKEI)�A���K�Ǘ����O(TURI_KAN)�A�R�C�����ɏW�v���O(COIN_SYU)�A�������ɏW�v���O(NOTE_SYU)
		// �o�[�W�����_�E���͍l�����Ȃ�

		// �v���O�����̃��O�o�[�W����
//		if( LOG_VERSION >= 5) // ���C���[�W
		{
			AppServ_Restore_ToV05(area, area_tbl, area_tbl_wk);
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	�o�b�N�A�b�v�f�[�^�i���O�o�[�W����5�j�̃��X�g�A����						|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = �o�b�N�A�b�v�f�[�^���							|*/
/*|				   area_tbl = �o�b�N�A�b�v�f�[�^���(�v���O����)			|*/
/*|				   area_tbl_wk = �o�b�N�A�b�v�f�[�^���(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_ToV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	// �o�b�N�A�b�v�̃��O�o�[�W����
	switch (bk_log_ver) {
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�i���O�o�[�W�����j
//	case 1:
//		AppServ_Restore_V01toV05(area, area_tbl, area_tbl_wk);
//		break;
//	case 2:
//	case 3:
//		AppServ_Restore_V02toV05(area, area_tbl, area_tbl_wk);
//		break;
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X�i���O�o�[�W�����j
	default:			// ���O�o�[�W�����s��(MH322902�ȑO)�A�܂��́A�o�[�W�����_�E��
		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�ƈ�v����΃��X�g�A����
		if (area_tbl->area[area].size == area_tbl_wk->area[area].size) {
			// �ʏ�̃��X�g�A����
			Flash2Read(area_tbl->area[area].address,
					area_tbl_wk->from_area[area].from_address,
					area_tbl_wk->area[area].size);
		}
		break;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���X�g�A�����iV01->V05�j												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = �o�b�N�A�b�v�f�[�^���							|*/
/*|				   area_tbl = �o�b�N�A�b�v�f�[�^���(�v���O����)			|*/
/*|				   area_tbl_wk = �o�b�N�A�b�v�f�[�^���(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_V01toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	COIN_SYU_V01 *pCoin_V01;
	COIN_SYU_V04 *pCoin_V04;
	NOTE_SYU_V01 *pNote_V01;
	NOTE_SYU_V04 *pNote_V04;
	TURI_KAN_V01 *pTurikan_V01;
	TURI_KAN_V04 *pTurikan_V04;
	int i;

	switch (area) {
	case OPE_DTNUM_COINSYU:			// �R�C�����ɏW�v
	case OPE_DTNUM_COIN_BK:			// �R�C�����ɍ��v�i�O�񕪁j
		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pCoin_V01 = (COIN_SYU_V01 *)g_TempUse_Buffer;
		pCoin_V04 = (COIN_SYU_V04 *)area_tbl->area[area].address;

		// COIN_SYU_V01�̃T�C�Y���R�s�[
		memcpy(pCoin_V04, pCoin_V01, sizeof(COIN_SYU_V01));
		// 0���Z�b�g
		pCoin_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_NOTESYU:			// �������ɏW�v
	case OPE_DTNUM_NOTE_BK:			// �������ɍ��v�i�O�񕪁j
		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pNote_V01 = (NOTE_SYU_V01 *)g_TempUse_Buffer;
		pNote_V04 = (NOTE_SYU_V04 *)area_tbl->area[area].address;

		// NOTE_SYU_V01�̃T�C�Y���R�s�[
		memcpy(pNote_V04, pNote_V01, sizeof(NOTE_SYU_V01));
		// 0���Z�b�g
		pNote_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_TURIKAN:			// ���K�Ǘ�
		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pTurikan_V01 = (TURI_KAN_V01 *)g_TempUse_Buffer;
		pTurikan_V04 = (TURI_KAN_V04 *)area_tbl->area[area].address;

		// turi_dat(TURI_KAN_V01)�̑O�܂ŃR�s�[
		memcpy(pTurikan_V04, pTurikan_V01, offsetof(TURI_KAN_V01, turi_dat));
		// TURI_kan_rec_V01�̃T�C�Y���R�s�[
		for (i = 0; i < 4; i++) {
			memcpy(&pTurikan_V04->turi_dat[i], &pTurikan_V01->turi_dat[i], sizeof(TURI_kan_rec_V01));
			pTurikan_V04->turi_dat[i].SeqNo = 0;
		}
		// sub_tube���Z�b�g
		pTurikan_V04->sub_tube = pTurikan_V01->sub_tube;
		// 0���Z�b�g
		pTurikan_V04->CenterSeqNo = 0;
		pTurikan_V04->SeqNo = 0;
		break;
	default:
		// �ʏ�̃��X�g�A����
		Flash2Read(area_tbl->area[area].address,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		break;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|	���X�g�A�����iV02->V05�j												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = �o�b�N�A�b�v�f�[�^���							|*/
/*|				   area_tbl = �o�b�N�A�b�v�f�[�^���(�v���O����)			|*/
/*|				   area_tbl_wk = �o�b�N�A�b�v�f�[�^���(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : T.Nagai													|*/
/*| Date         : 2015/01/09												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_V02toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	COIN_SYU_V01 *pCoin_V01;
	COIN_SYU_V04 *pCoin_V04;
	NOTE_SYU_V01 *pNote_V01;
	NOTE_SYU_V04 *pNote_V04;
	TURI_KAN_V02 *pTurikan_V02;
	TURI_KAN_V04 *pTurikan_V04;
	int i;

	switch (area) {
	case OPE_DTNUM_COINSYU:			// �R�C�����ɏW�v
	case OPE_DTNUM_COIN_BK:			// �R�C�����ɍ��v�i�O�񕪁j
		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pCoin_V01 = (COIN_SYU_V01 *)g_TempUse_Buffer;
		pCoin_V04 = (COIN_SYU_V04 *)area_tbl->area[area].address;

		// COIN_SYU_V01�̃T�C�Y���R�s�[
		memcpy(pCoin_V04, pCoin_V01, sizeof(COIN_SYU_V01));
		// 0���Z�b�g
		pCoin_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_NOTESYU:			// �������ɏW�v
	case OPE_DTNUM_NOTE_BK:			// �������ɍ��v�i�O�񕪁j
		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pNote_V01 = (NOTE_SYU_V01 *)g_TempUse_Buffer;
		pNote_V04 = (NOTE_SYU_V04 *)area_tbl->area[area].address;

		// NOTE_SYU_V01�̃T�C�Y���R�s�[
		memcpy(pNote_V04, pNote_V01, sizeof(NOTE_SYU_V01));
		// 0���Z�b�g
		pNote_V04->SeqNo = 0;
		break;
	case OPE_DTNUM_TURIKAN:			// ���K�Ǘ�
		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
		Flash2Read(g_TempUse_Buffer,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		pTurikan_V02 = (TURI_KAN_V02 *)g_TempUse_Buffer;
		pTurikan_V04 = (TURI_KAN_V04 *)area_tbl->area[area].address;

		// turi_dat(TURI_KAN_V02)�̑O�܂ŃR�s�[
		memcpy(pTurikan_V04, pTurikan_V02, offsetof(TURI_KAN_V02, turi_dat));
		// TURI_kan_rec_V01�̃T�C�Y���R�s�[
		for (i = 0; i < 4; i++) {
			memcpy(&pTurikan_V04->turi_dat[i], &pTurikan_V02->turi_dat[i], sizeof(TURI_kan_rec_V01));
			// 0���Z�b�g
			pTurikan_V04->turi_dat[i].SeqNo = 0;
		}
		// sub_tube�`CenterSeqNo���Z�b�g
		pTurikan_V04->sub_tube = pTurikan_V02->sub_tube;
		pTurikan_V04->CenterSeqNo = pTurikan_V02->CenterSeqNo;
		// 0���Z�b�g
		pTurikan_V04->SeqNo = 0;
		break;
	default:
		// �ʏ�̃��X�g�A����
		Flash2Read(area_tbl->area[area].address,
				area_tbl_wk->from_area[area].from_address,
				area_tbl_wk->area[area].size);
		break;
	}
}

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
/*[]-----------------------------------------------------------------------[]*/
/*|	�W�v���O���X�g�A����													|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = �o�b�N�A�b�v�f�[�^���							|*/
/*|				   area_tbl = �o�b�N�A�b�v�f�[�^���(�v���O����)			|*/
/*|				   area_tbl_wk = �o�b�N�A�b�v�f�[�^���(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanizaki												|*/
/*| Date         : 2019/05/29												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_SYUKEI(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong		ulBuckupLogSize;
	uchar		*pSkyBuckup = NULL;
	struct SKY	*pSkyRestore = NULL;

	switch (area) {
	case OPE_DTNUM_SKY:			// �W�v�iT,GT,������j
	case OPE_DTNUM_SKYBK:		// �O��W�v

		if(bk_log_ver == 0 || bk_log_ver == 0xFF) {
			// LOG_VERSION���s��(MH322902�ȑO)�̏ꍇ�͏W�v�G���A�̍\�����傫���قȂ邽�߁A
			// ���X�g�A�̑ΏۂƂ��Ȃ�
			// 0xFF��FROM�̏����l�ALOG_VERSION����`����Ă��Ȃ��o�[�W���������
			// �o�[�W�����A�b�v���͂��̒l�������Ă���
		} else {
			// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
			Flash2Read(g_TempUse_Buffer,
					area_tbl_wk->from_area[area].from_address,
					area_tbl_wk->area[area].size);
			pSkyBuckup = (uchar*)g_TempUse_Buffer;
			pSkyRestore = (struct SKY*)area_tbl->area[area].address;

			// �o�b�N�A�b�v�̊e�W�v���O�T�C�Y���Z�o
			ulBuckupLogSize = area_tbl_wk->area[area].size / 4;

			// �R�s�[��̃G���A���N���A
			memset(area_tbl->area[area].address, 0, area_tbl->area[area].size);

			// �W�v���O���R�s�[
			memcpy(&pSkyRestore->tsyuk, pSkyBuckup, ulBuckupLogSize);	// T�W�v
			pSkyBuckup += ulBuckupLogSize;
			memcpy(&pSkyRestore->gsyuk, pSkyBuckup, ulBuckupLogSize);	// GT�W�v
			pSkyBuckup += ulBuckupLogSize;
			memcpy(&pSkyRestore->msyuk, pSkyBuckup, ulBuckupLogSize);	// MT�W�v
			pSkyBuckup += ulBuckupLogSize;
			memcpy(&pSkyRestore->fsyuk, pSkyBuckup, ulBuckupLogSize);	// �����䐔�W�v
		}
		break;
	default:	// �Ăяo�����Ő������Ă��邯�ǔO�̂���
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|	�����W�v���O���X�g�A����(Suica,Dey,EC)									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = �o�b�N�A�b�v�f�[�^���							|*/
/*|				   area_tbl = �o�b�N�A�b�v�f�[�^���(�v���O����)			|*/
/*|				   area_tbl_wk = �o�b�N�A�b�v�f�[�^���(Flash)				|*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Y.Tanizaki												|*/
/*| Date         : 2019/05/29												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void AppServ_Restore_SYUKEISP(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	switch (area) {
	case OPE_DTNUM_LOG_DCSYUUKEI:
// MH810103 GG119202(S) �ʃp�����[�^�����X�g�A����Ȃ�
//		// �o�b�N�A�b�v���̃f�[�^�T�C�Y�œǍ���
//		Flash2Read(area_tbl->area[area].address,
//				area_tbl_wk->from_area[area].from_address,
//				area_tbl_wk->area[area].size);
// MH810103(s) �����W�v�����X�g�A����Ȃ��s��C��
//		// �����W�v�f�[�^�\���̂̃T�C�Y�����������߁A1�Z�N�^�ł̓o�b�N�A�b�v�ł��Ȃ�
//		// ���̂��߁A�����W�v�f�[�^�̃o�b�N�A�b�v�ʒu��OPE_DTNUM_LOG_DCSYUUKEI_EC�Ƃ����B
//
//		// �ELOG_VERSION=1�ȑO��OPE_DTNUM_LOG_DCSYUUKEI����
//		//   OPE_DTNUM_LOG_DCSYUUKEI_EC�փ��X�g�A����i�擪��sca_inf�̂݁j
//		// �ELOG_VERSION=2�ȍ~��area_tbl_wk->from_area[OPE_DTNUM_LOG_DCSYUUKEI].from_address��
//		//   0�̂��߁A���̊֐��̓R�[������Ȃ��B
//		if (bk_log_ver <= 1) {
//			Flash2Read(area_tbl->area[OPE_DTNUM_LOG_DCSYUUKEI_EC].address,
//					area_tbl_wk->from_area[area].from_address,
//					sizeof(syuukei_info));
//		}
		// �����W�v�f�[�^�̃o�b�N�A�b�v�ʒu��OPE_DTNUM_LOG_DCSYUUKEI_EC�ɕύX
		// �T�C�Y�͕ύX�Ȃ�

		// �ELOG_VERSION=1�ȑO��OPE_DTNUM_LOG_DCSYUUKEI����
		//   OPE_DTNUM_LOG_DCSYUUKEI_EC�փ��X�g�A����
		// �ELOG_VERSION=2�ȍ~��area_tbl_wk->from_area[OPE_DTNUM_LOG_DCSYUUKEI].from_address��
		//   0�̂��߁A���̊֐��̓R�[������Ȃ��B
		if (bk_log_ver <= 1) {
			Flash2Read(area_tbl->area[OPE_DTNUM_LOG_DCSYUUKEI_EC].address,
					area_tbl_wk->from_area[area].from_address,
					area_tbl_wk->area[area].size);
		}
// MH810103(e) �����W�v�����X�g�A����Ȃ��s��C��
// MH810103 GG119202(E) �ʃp�����[�^�����X�g�A����Ȃ�
		break;
	default:	// �Ăяo�����Ő������Ă��邯�ǔO�̂���
		break;
	}
}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810105 MH321800(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
/*[]-----------------------------------------------------------------------[]*/
/*|	��������������O���X�g�A����(���σ��[�_)									|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : area = �o�b�N�A�b�v�f�[�^���							   |*/
/*|				   area_tbl_wk = �o�b�N�A�b�v�f�[�^���(Flash)				  |*/
/*| RETURN VALUE : 															|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : D.Inaba										�@�@		|*/
/*| Date         : 2021/03/22												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2021 AMANO Corp.---[]*/
void AppServ_Restore_ECALAMLOG(ushort area, t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong	FromAdrs;
	ulong	datasize;
	ulong	wk_paydatasize = 0;

	switch (area) {
	case OPE_DTNUM_EC_ALARM_TRADE_LOG:
		memset( &EC_ALARM_LOG_DAT, 0, sizeof(EC_ALARM_LOG_DAT) );					// ���X�g�A��O�N���A

// MH810105 GG119202(S) ������������L�^���X�g�A�����C��
//		// ����������O�����X�g�A����
//		FromAdrs = area_tbl_wk->from_area[area].from_address;						// �擪�A�h���X�擾
//		datasize = _offsetof(Ec_Alarm_Log_rec, paylog);								// �Đ��Z���O�܂ł̃f�[�^�T�C�Y�擾
//		Flash2Read( (uchar*)&EC_ALARM_LOG_DAT, FromAdrs, datasize );				// ����������O�@���X�g�A
		// ����������O�̃w�b�_�������X�g�A����
		FromAdrs = area_tbl_wk->from_area[area].from_address;						// �擪�A�h���X�擾
		datasize = _offsetof(Ec_Alarm_Log_rec, log);								// ����������O�܂ł̃f�[�^�T�C�Y�擾
		Flash2Read( (uchar*)&EC_ALARM_LOG_DAT, FromAdrs, datasize );				// �w�b�_��񃊃X�g�A

		// �������������ꍇ�͖���������A�Đ��Z���̃��X�g�A�s��
		if (EC_ALARM_LOG_DAT.array_cnt > EC_ALARM_LOG_CNT) {
			memset(&EC_ALARM_LOG_DAT, 0, datasize);
			EC_ALARM_LOG_DAT.array_cnt = EC_ALARM_LOG_CNT;
			EC_ALARM_LOG_DAT.data_size = sizeof(t_ALARM_SETTLEMENT_DATA);
			break;
		}

		// ��������������X�g�A����
		FromAdrs += datasize;
		datasize = EC_ALARM_LOG_DAT.data_size * EC_ALARM_LOG_DAT.array_cnt;
		Flash2Read((uchar*)EC_ALARM_LOG_DAT.log, FromAdrs, datasize);				// ���������񃊃X�g�A
// MH810105 GG119202(E) ������������L�^���X�g�A�����C��

		// LOG_VERSION>=2�̎��͍Đ��Z��񃍃O�������X�g�A���������߁A��x�����Ő��Z���O�T�C�Y��ǂݏo��
		if( bk_log_ver >= 2 ){
			// �{���ł���΁AFLT_ReadBRLOGSZ( &BR_LOG_SIZE[0] );��胍�O�T�C�Y���擾��������
			// �N�����̕��d���������A���łɃ��O�T�C�Y��V�������O�T�C�Y�ɏ��������Ă��邻�̂��߁A
			// �o�b�N�A�b�v�e�[�u���ɂ��関��������O����ߋ��̐��Z���O�T�C�Y���v�Z����B
			// ��AppServ_FukudenProc��FLT_init�ɂă��O�T�C�Y��V�������Ă���B
			wk_paydatasize = (area_tbl_wk->area[area].size - datasize) / EC_ALARM_LOG_DAT.array_cnt;

			// ���Z���O�T�C�Y�ɍ��ق�����ꍇ�͍Đ��Z���̃��X�g�A�s��
			if ( wk_paydatasize == LogDatMax[eLOG_PAYMENT][0] ) {
				// �Đ��Z�������X�g�A����
				FromAdrs += datasize;
				datasize = sizeof(Receipt_data) * EC_ALARM_LOG_DAT.array_cnt;
				Flash2Read( (uchar*)EC_ALARM_LOG_DAT.paylog, FromAdrs, datasize );		// �Đ��Z��񃊃X�g�A
			}
		}

		EC_ALARM_LOG_DAT.array_cnt = EC_ALARM_LOG_CNT;
		EC_ALARM_LOG_DAT.data_size = sizeof(t_ALARM_SETTLEMENT_DATA);
		break;
	default:	// �Ăяo�����Ő������Ă��邯�ǔO�̂���
		break;
	}
}
// MH810105 MH321800(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
