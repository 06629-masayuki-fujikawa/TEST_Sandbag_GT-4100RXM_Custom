#ifndef	_APPSERV_H_
#define	_APPSERV_H_
/*[]-----------------------------------------------------------------------[]*/
/*| Backup/Restore, Download/Upload common include                          |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : Okuda                                                    |*/
/*| Date         : 2005/06/24                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*----------------------------------------------------------*/
/*	�p�����[�^�f�[�^���e�[�u�� 							*/
/*----------------------------------------------------------*/

/* �Z�N�V�������̏�� */
typedef struct {
	void				*address;		/* �Z�N�V�����擪�A�h���X */
	ushort				item_num;		/* �Z�N�V�������̍��ڐ� */
}t_AppServ_SectInfo;

/* �p�����[�^���e�[�u�� */
typedef struct {
	ushort				csection_num;	/* ���ʃp�����[�^ �Z�N�V������ */
	ushort				psection_num;	/* �ʃp�����[�^�Z�N�V������ */
	t_AppServ_SectInfo	csection[64];	/* ���ʃp�����[�^ */
	t_AppServ_SectInfo	psection[64];	/* �ʃp�����[�^ */
}t_AppServ_ParamInfoTbl;

// �����Ұ���߰�����ύX
extern	ushort	AppServ_PParam_Copy;			// FLASH �� RAM �����Ұ���߰�L��
												//	OFF:�����Ұ����߰���Ȃ�
												//	ON :�����Ұ����߰����

/*----------------------------------------------------------*/
/*	�o�b�N�A�b�v�f�[�^���e�[�u�� 							*/
/*----------------------------------------------------------*/

/* �G���A���̏�� */
typedef struct {
	void				*address;		/* �G���A�擪�A�h���X */
	ulong				size;			/* �G���A�T�C�Y */
}t_AppServ_AreaInfo;

typedef struct {
	ulong				from_address;	// �o�b�N�A�b�v��FROM�̐擪�A�h���X
}t_AppServ_FromInfo;

/* �G���A���e�[�u�� */
typedef struct {
	ushort				area_num;	/* �G���A�� */
	t_AppServ_AreaInfo	area[64];	/* �e�G���A���Ƃ̏�� */
	t_AppServ_FromInfo	from_area[64];
} t_AppServ_AreaInfoTbl;


/*----------------------------------------------------------*/
/*	�p�����[�^Upload/Download�p �ۑ��f�[�^					*/
/*----------------------------------------------------------*/

typedef struct {			/* BIN��CSV�ϊ������̕ۑ��f�[�^ */
	ulong 					datasize;			/* �Z�b�g�o�b�t�@�T�C�Y 						*/
												/* (���������ɗv��������n�����)				*/
	uchar					Phase;				/* �ϊ�������� 0=���o���Z�b�g���A1=�f�[�^�Z�b�g�� */
	uchar					Kind;				/* �Ώۃp�����[�^�w��i0=���ʁC1=�ʁj			*/
	ushort					Section;			/* ����������Z�N�V�����ԍ��i0�`�j				*/
	ushort					Item;				/* ����������Z�N�V�������A�C�e���ԍ��i0�`�j	*/
	ushort					CallCount;			/* Call���ꂽ��								*/
} t_AppServ_Bin_toCsv_Param;

typedef struct {			/* CSV��BIN�ϊ������̕ۑ��f�[�^ */
	char					*param_image;		/* �o�C�i���C���[�W�ۑ��G���A�ւ̃|�C���^ 		*/
												/* (���������ɗv��������n�����)				*/
	ulong					image_size;			/* param_image�G���A�̃T�C�Y 					*/
												/* (���������ɗv��������n�����)				*/
	ushort					CallCount;			/* Call���ꂽ��								*/
//	ulong					WriteIndex;			/* �������݈ʒu(offset) 						*/
} t_AppServ_Csv_toBin_Param;

typedef struct {			/* BIN,CSV�ϊ������̕ۑ��f�[�^ */
	t_AppServ_ParamInfoTbl 	param_info;			/* �p�����[�^���e�[�u���ւ̃|�C���^ 			*/
												/* BIN��CSV�CCSV��BIN�ǂƂ�������f�[�^�Ȃ̂� 	*/
												/* ��җD��ŕۑ�����							*/
	t_AppServ_Bin_toCsv_Param	Bin_toCsv_Param;
	t_AppServ_Csv_toBin_Param	Csv_toBin_Param;
} t_AppServ_BinCsvSave;

/*----------------------------------------------------------*/
/*	�Ԏ��p�����[�^�������ݗp �ۑ��f�[�^						*/
/*----------------------------------------------------------*/

typedef struct {
	uchar	sts;
	char	*image;
}t_AppServ_LockInfoFukuden;

/*----------------------------------------------------------*/
/*	�`�o�h�O���Q�Ɓi�v���g�^�C�v�j							*/
/*----------------------------------------------------------*/
extern	void	AppServ_MakeBackupDataInfoTable( t_AppServ_AreaInfoTbl *pTable );
extern	void	AppServ_MakeParamInfoTable( t_AppServ_ParamInfoTbl *pTable );
extern	void	AppServ_FormatHeaderFooter( char formated[4][36], const char *text, ushort text_len );
// �����X�_�E�����[�h���AAppServ_FormatSyomeiKamei()�ł͂Ȃ�ppServ_FormatHeaderFooter���g�p���Ă���
// �������e��HeaderFooter�ƑS�������Ȃ̂œ�������
#define	AppServ_FormatSyomeiKamei		AppServ_FormatHeaderFooter
#define	AppServ_InsertCrlfToSyomeiKamei	AppServ_InsertCrlfToHeaderFooter

extern	ushort	AppServ_InsertCrlfToEdyAtCommand( char *inserted, const char text[6][20] );
extern	void	AppServ_FormatEdyAtCommand( char formated[6][20], const char *text, ushort text_len );

extern	ushort	AppServ_InsertCrlfToHeaderFooter( char *inserted, const char text[4][36] );
extern	uchar	AppServ_ConvParam2CSV_Init( const t_AppServ_ParamInfoTbl *param_info, ulong datasize );
extern	void	AppServ_CnvParam2CSV( char *csvdata, ushort *csvdata_len, uchar *finish );
extern	uchar	AppServ_ConvCSV2Param_Init( const t_AppServ_ParamInfoTbl *param_info, 
									char *param_image, ulong image_size );
extern	uchar	AppServ_CnvCSV2Param(const char *csvdata, ushort csvdata_len, ulong *finish );
extern	ulong	AppServ_ConvCsvParam_ImageAreaSizeGet( const t_AppServ_ParamInfoTbl *param_info );
extern	void	AppServ_FukudenProc( char RAMCLR, char f_CPrmDefaultSet );
extern	void	DataSumUpdate( ushort kind );
extern	uchar	DataSumCheck( ushort kind );
extern	void	AppServ_LogFlashWriteReq( short id );
extern	uchar	AppServ_IsLogFlashWriting( short id );

extern	uchar	AppServ_CnvCSV2LockInfo_Init( char *lock_info_image, ulong image_size );
extern	uchar	AppServ_CnvCSV2LockInfo(const char *csvdata, ushort csvdata_len, ulong *finish );
extern	uchar	AppServ_CnvLockInfo2CSV_Init( void );
extern	void	AppServ_CnvLockInfo2CSV( char *csvdata, ushort *csvdata_len, uchar *finish );
extern	void	AppServ_ConvErrArmCSV( char *csvdata, ulong *csvdata_len, uchar *finish );
extern	void	AppServ_ConvErrArmCSV_Init( uchar kind );
extern	uchar	AppServ_CnvCSVtoFLAPDT_Init( char *, ulong );
extern	uchar	AppServ_CnvCSVtoFLAPDT(const char *, ushort, ulong * );
extern	ushort	AppServ_InsertCrlfToTextData( char *inserted, const char text[][36], uchar line_num );
extern	void	AppServ_FormatTextData( char formated[][36], const char *text, ushort text_len, uchar line_num );
extern	void	AppServ_SaleTotal_LOG_edit( char *, ulong *, uchar * );
extern	void	AppServ_SaleTotal_LOG_Init( uchar, ulong );
extern	void	AppServ_ConvChkResultCSV( char *csvdata, ulong *csvdata_len, uchar *finish );
extern	void 	Get_Result_dat(uchar no, uchar cnt, uchar *dat);
extern	void	AppServ_ConvChkResultCSV_Init(void);
extern	void	AppServ_Restore_BackupData(const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_ToV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_V01toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_V02toV05(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
extern	void	AppServ_Restore_SYUKEI(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
extern	void	AppServ_Restore_SYUKEISP(ushort area, const t_AppServ_AreaInfoTbl *area_tbl, t_AppServ_AreaInfoTbl *area_tbl_wk);
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810105 MH321800(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
extern	void	AppServ_Restore_ECALAMLOG(ushort area, t_AppServ_AreaInfoTbl *area_tbl_wk);
// MH810105 MH321800(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810100(S) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
extern	ushort	AppServ_CnvCsvParam_SkipItemCount_tillTargetSecterGet( char SecKind, ushort SecNum );
// MH810100(E) K.Onodera 2019/11/29 �Ԕԃ`�P�b�g���X(�����e�i���X)

#endif
