/*[]----------------------------------------------------------------------[]
 *| �O��FROM2�Ǘ����W���[��
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.07.27
 *| Update      :
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"L_FLASHROM.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"remote_dl.h"
#include	"FlashSerial.h"
#include	"updateope.h"
#include	"rauconstant.h"
#include	"raudef.h"
#include	"ftpctrl.h"


/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/
/* �f�[�^�T�C�Y */

#define	FLT_PARAM_DATA_SIZE			sizeof(long)

#define	FLT_LOGOFS_RECORD_NUM		2
#define	FLT_LOGOFS_SEQNO			4
#define	FLT_LOGOFS_RECORD			6	/* �擪���R�[�h�ʒu */

#define	FLT_LKPRMOFS_LOCKMAX		2
#define	FLT_LKPRMOFS_DATASIZE		4
#define	FLT_LKPRMOFS_PARAM			6

#define	FLT_PRINTOFS_LOGO			0
#define	FLT_PRINTOFS_HEADER			(FLT_PRINTOFS_LOGO+FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE)
#define	FLT_PRINTOFS_FOOTER			(FLT_PRINTOFS_HEADER+FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE)

#define	FLT_PRINTOFS_CREDIT1		(FLT_PRINTOFS_FOOTER+FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE)
#define	FLT_PRINTOFS_CREDIT2		(FLT_PRINTOFS_CREDIT1+FLT_HEADER_SIZE+FLT_CREDIT_SIZE)
#define	FLT_PRINTOFS_ACCEPTFOOTER	(FLT_PRINTOFS_CREDIT2+FLT_HEADER_SIZE+FLT_CREDIT_SIZE)

#define	FLT_PRINTOFS_SYOMEI			(FLT_PRINTOFS_ACCEPTFOOTER+FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE)
#define	FLT_PRINTOFS_KAMEI			(FLT_PRINTOFS_SYOMEI+FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE)

#define	FLT_PRINTOFS_EDYAT			(FLT_PRINTOFS_KAMEI+FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE)

#define	FLT_PRINTOFS_USERDEF	_offsetof(_PRNSECT_IMG,userdef)

#define	FLT_PRINTOFS_TCARDFTR		_offsetof(_PRNSECT_IMG,tcardftr)
#define	FLT_PRINTOFS_AZUFTR			_offsetof(_PRNSECT_IMG,azuftr)
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
#define	FLT_PRINTOFS_CREKBRFTR			_offsetof(_PRNSECT_IMG,crekbrftr)
#define	FLT_PRINTOFS_EPAYKBRFTR			_offsetof(_PRNSECT_IMG,epaykbrftr)
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
#define	FLT_PRINTOFS_FUTUREFTR			_offsetof(_PRNSECT_IMG,futureftr)
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define	FLT_PRINTOFS_EMGFOOTER		_offsetof(_PRNSECT_IMG,emgfooter)
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
#define	_ETC_SIZE_		((ulong)_offsetof(_PRNSECT_IMG,end))

/* �߂�l�쐬 */
#define	_FLT_MkRtnVal(result, detail)	(((ulong)(result) << 8*3) + ((ulong)(detail) & 0x00FFFFFF))
#define	_FLT_MkRtnVal2(result)			_FLT_MkRtnVal((result), 0)

/* t_FltReq::req */
enum {
	FLT_REQ_IDLE,		/* �v������t */
	FLT_REQ_WRITE,		/* �������ݗv����t�ς�(�����s) */
	FLT_REQ_EXEC_WRITE,	/* �������ݎ��s�� */
	FLT_REQ_READ		/* �ǂݍ��ݎ��s�� */
};

/* flt_WriteParam1()���� */
#define	FLT_CMP_ALL			0	/* ���ʁA�ʃp�����[�^�Ƃ���r���s�� */
#define	FLT_CMP_CPRM_ONLY	1	/* ���ʃp�����[�^�̂ݔ�r���s�� */

#define	FLT_DTNUM_MAPVER		OPE_DTNUM_COMPARA	/* FLASH������MAP�o�[�W�����ʒu	 */
#define	FLT_DTNUM_VERUPFLG		OPE_DTNUM_PERPARA	/* �o�[�W�����A�b�v�t���O�ʒu */

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/

extern t_FLT_Ctrl_Backup FLT_Ctrl_Backup;
extern t_FLT_Ctrl_NotBackup FLT_Ctrl_NotBackup;

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

static	ulong	FLT_PARAM_SECTOR;

extern	char	FLT_f_TaskStart;						/* FlashROM�^�X�N�N���t���O */
														/* 0=�N���O�A1=�N����		*/
/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

/*--- for initialization ---*/
static void		flt_Fukuden(void);
static void		flt_ChkFlashData(t_FltDataInfo *data_info, 
								uchar *param_ok, 
								uchar *lockparam_ok, 
								ulong *ver_flg);
static void		flt_ChkLog(t_FltLogMgr *log_mgr);
static BOOL		flt_CheckAreaSum(ulong top_address, ulong sectsize, ushort sectnum);
static void		flt_ChkPrintData(uchar *wbuf, t_FltReq *req_tbl);
static BOOL		flt_CheckData(ulong top_address, ulong size, BOOL *blank);
static BOOL		flt_CheckSum(ulong top_address, ulong size);
/*--- write operations ---*/
static ulong	flt_WriteParam1(const t_AppServ_ParamInfoTbl *param_tbl, 
						t_AppServ_ParamInfoTbl *param_tbl_wk, 
						uchar *wbuf, 
						t_FltReq *req_tbl, 
						uchar cmp_method);
static ulong	flt_CmpParamData(const t_AppServ_ParamInfoTbl *param_tbl, 
						t_AppServ_ParamInfoTbl *param_tbl_wk, 
						uchar cmp_method);
static ulong	flt_WriteParam2(const t_FltParam_Param2 *params, 
						const t_AppServ_ParamInfoTbl *param_tbl, 
						uchar *wbuf,
						t_FltReq *req_tbl);
static ulong	flt_WriteLog(t_FltParam_Log *params, t_FltLogMgr *logmgr, t_FltReq *req_tbl, short clr_kind);
static ulong	flt_WriteLogData(ushort Lno, const t_FltParam_Log *params, const t_FltLogMgr *logmgr, ushort sector, ushort seqno);
static void		flt_GetWriteLogSector(const t_FltLogMgr *logmgr, ushort *sector, ushort *seqno);
static ulong	flt_CmpLogData(ushort Lno, const t_FltParam_Log *params, const t_FltLogMgr *logmgr, ushort sector, ushort seqno);
static ulong	flt_WriteLogo(const t_FltParam_Logo *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_WriteHeader(const t_FltParam_Header *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_WriteFooter(const t_FltParam_Footer *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_WriteAcceptFooter(const t_FltParam_AcceptFooter *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_WriteSyomei(const t_FltParam_Syomei *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_WriteKamei(const t_FltParam_Kamei *params, uchar *wbuf, t_FltReq *req_tbl);
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY 
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
static ulong	flt_WriteEdyAt(const t_FltParam_EdyAt *params, uchar *wbuf, t_FltReq *req_tbl);
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#endif
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

static ulong	flt_WriteUserDef(const t_FltParam_UserDef *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_ReadUserDef(char *buf);
static	ulong	flt_WriteEtc(const t_FltParam_Etc *params, t_FltReq *req_tbl);
static	ulong	flt_WriteImage(const t_FltParam_Image *params, t_FltReq *req_tbl);
static ulong	flt_WriteParam3(const t_FltParam_Param2 *params, 
						const t_AppServ_ParamInfoTbl *param_tbl, 
						uchar *wbuf,
						t_FltReq *req_tbl);

static ulong	flt_WriteSector(ulong address, uchar *data, ulong size);
static ulong	flt_WriteParamSector(ulong address, uchar *data, ulong size);
static ulong	flt_WritePrintDataSector(uchar *data);
static ulong	flt_WriteLockParam1(uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_CmpLockParamData(void);
static ulong	flt_WriteLockParam2(const t_FltParam_LockParam2 *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_Backup(
						const t_FltParam_Bkup *bkup_params,
						const t_AppServ_ParamInfoTbl *param_tbl, 
						t_AppServ_ParamInfoTbl *param_tbl_wk,
						const t_AppServ_AreaInfoTbl *area_tbl,
						t_AppServ_AreaInfoTbl *area_tbl_wk,
						uchar *wbuf);
static ulong	flt_WriteBackupData(const t_AppServ_AreaInfoTbl *area_tbl,
								t_AppServ_AreaInfoTbl *area_tbl_wk,
								const t_AppServ_ParamInfoTbl *param_tbl,
								ulong ver_flg);
static ulong	flt_CmpBackupData(const t_AppServ_AreaInfoTbl *area_tbl, 
						t_AppServ_AreaInfoTbl *area_tbl_wk,
						const t_AppServ_ParamInfoTbl *param_tbl,
						ulong ver_flg);
static ulong	flt_WriteBackupDataSector(const t_AppServ_AreaInfoTbl *area_tbl, const t_AppServ_ParamInfoTbl *param_tbl, ulong ver_flg);
static ulong	flt_WritePParamTbl(const t_AppServ_ParamInfoTbl *param_tbl, ulong addr);
static ulong	flt_WritePParamData(const t_AppServ_ParamInfoTbl *param_tbl, ulong addr);
static ulong	flt_WriteAreaData(const t_AppServ_AreaInfoTbl *area_tbl);
static ulong	flt_Write(ulong address, uchar *data, ulong size);
static void		flt_SetSum(uchar *buf, ulong size);
static uchar	flt_CalcSum(uchar *data, ulong cnt);
/*--- read operations ---*/
static ulong	flt_RestoreParam(const t_AppServ_ParamInfoTbl *param_tbl, t_AppServ_ParamInfoTbl *param_tbl_wk);
static ulong	flt_RestoreLockParam(void);
static long		flt_GetLogRecordNum(const t_FltLogMgr *logmgr);
static ulong	flt_ReadLogRecord(const t_FltLogMgr *logmgr, const t_FltLogHandle *handle, char *buf);
static ulong	flt_NextLog(const t_FltLogMgr *logmgr, t_FltLogHandle *handle);
static ulong	flt_ReadLogo(char *buf, char RaedType, ushort TopOffset, ushort ReadSize);

static ulong	flt_ReadHeader(char *buf);
static ulong	flt_ReadFooter(char *buf);
static ulong	flt_ReadAcceptFooter(char *buf);
static ulong	flt_ReadSyomei(char *buf);
static ulong	flt_ReadKamei(char *buf);

static ulong	flt_Restore(const t_AppServ_ParamInfoTbl *param_tbl, 
					t_AppServ_ParamInfoTbl *param_tbl_wk,
					const t_AppServ_AreaInfoTbl *area_tbl, 
					t_AppServ_AreaInfoTbl *area_tbl_wk);
static BOOL		flt_GetVersion(ulong *mapver, ulong *ver_flg);
static ulong	flt_Restore_BackupData(const t_AppServ_AreaInfoTbl *area_tbl, 
									t_AppServ_AreaInfoTbl *area_tbl_wk,
									const t_AppServ_ParamInfoTbl *param_tbl);
/*--- table operations ---*/
static ulong	flt_WriteParamTbl(uchar *buf, const t_AppServ_ParamInfoTbl *param_tbl);
static void		flt_ReadParamTbl(t_AppServ_ParamInfoTbl *param_tbl_wk);
static void		flt_ReadParamTblFrom(t_AppServ_ParamInfoTbl *param_tbl_wk, ulong sect_addr);
static BOOL		flt_ParamCopy(uchar *dst, const t_AppServ_ParamInfoTbl *param_tbl);
static ulong	flt_ParamTblSize(const t_AppServ_ParamInfoTbl *param_tbl);
static ulong	flt_WriteAreaTbl(const t_AppServ_AreaInfoTbl *area_tbl, ulong ver_flg);
static void		flt_ReadAreaTbl(t_AppServ_AreaInfoTbl *area_tbl_wk);
#define	_flt_PParamTblSize(tbl)	(sizeof((tbl)->psection_num) + sizeof((tbl)->psection[0]) * (tbl)->psection_num)
static ulong	flt_CParamDataSize(const t_AppServ_ParamInfoTbl *param_tbl);

static ulong	flt_ReadAzuFtr(char *buf);
static ulong	flt_WriteAzuFtr(const t_FltParam_AzuFtr *params, uchar *wbuf, t_FltReq *req_tbl);
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
static ulong	flt_ReadCreKbrFtr(char *buf);
static ulong	flt_WriteCreKbrFtr(const t_FltParam_CreKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_ReadEpayKbrFtr(char *buf);
static ulong	flt_WriteEpayKbrFtr(const t_FltParam_EpayKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl);
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
static ulong	flt_ReadFutureFtr(char *buf);
static ulong	flt_WriteFutureFtr(const t_FltParam_FutureFtr *params, uchar *wbuf, t_FltReq *req_tbl);
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
static ulong	flt_ReadEmgFooter(char *buf);
static ulong	flt_WriteEmgFooter(const t_FltParam_EmgFooter *params, uchar *wbuf, t_FltReq *req_tbl);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
static ulong	flt_ReadBRLOGSZ( ushort *buf );
static ulong	flt_WriteBRLOGSZ(const t_FltParam_BRLOGSZ *params, uchar *wbuf, t_FltReq *req_tbl);

/*--- utilities ---*/
static ulong	flt_EraseSectors(ulong top, ulong sector_size, ulong sector_num);
static BOOL		flt_LockFlash(t_FltReq *req_tbl, uchar kind, uchar req, uchar flag);
static void		flt_UnlockFlash(t_FltReq *req_tbl, uchar kind);
static BOOL		flt_CheckBusy(const t_FltReq *req_tbl, uchar kind, uchar req, uchar flag);

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/
// Usage of Flash Memory 2
#define	_A_ARCH	(0x20)
#define	_A_SYS	(0x04)
#define	_TYP_DATA		(_A_ARCH)
#define	_TYP_SYSTEM		(_A_ARCH+_A_SYS)

static	struct {
		ushort		typ;
		ushort		num;
		ulong		top;
		ulong		size;
	} from2_map[] = {
	{_TYP_DATA,		FLT_LOG0_SECTORMAX,			FLT_LOG0_SECTOR,		sizeof(Receipt_data)},
// MH810100(S) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
//	{_TYP_DATA,		FLT_LOG1_SECTORMAX,			FLT_LOG1_SECTOR,		sizeof(enter_log)},
	{_TYP_DATA,		FLT_LOG1_SECTORMAX,			FLT_LOG1_SECTOR,		sizeof(RTPay_log)},
// MH810100(E) K.Onodera 2019/11/25 �Ԕԃ`�P�b�g���X (RT���Z�f�[�^�Ή�)
	{_TYP_DATA,		FLT_LOG2_SECTORMAX,			FLT_LOG2_SECTOR,		sizeof(SYUKEI)},
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	{_TYP_DATA,		FLT_LOG3_SECTORMAX,			FLT_LOG3_SECTOR,		sizeof(LCKTTL_LOG)},
	{_TYP_DATA,		FLT_LOG3_SECTORMAX,			FLT_LOG3_SECTOR,		sizeof(RTReceipt_log)},
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	{_TYP_DATA,		FLT_LOG4_SECTORMAX,			FLT_LOG4_SECTOR,		sizeof(Err_log)},
	{_TYP_DATA,		FLT_LOG5_SECTORMAX,			FLT_LOG5_SECTOR,		sizeof(Arm_log)},
	{_TYP_DATA,		FLT_LOG6_SECTORMAX,			FLT_LOG6_SECTOR,		sizeof(Ope_log)},
	{_TYP_DATA,		FLT_LOG7_SECTORMAX,			FLT_LOG7_SECTOR,		sizeof(Mon_log)},
	{_TYP_DATA,		FLT_LOG8_SECTORMAX,			FLT_LOG8_SECTOR,		sizeof(flp_log)},
	{_TYP_DATA,		FLT_LOG9_SECTORMAX,			FLT_LOG9_SECTOR,		sizeof(TURI_KAN)},
	{_TYP_DATA,		FLT_LOG10_SECTORMAX,		FLT_LOG10_SECTOR,		sizeof(ParkCar_log)},
	{_TYP_DATA,		FLT_LOG11_SECTORMAX,		FLT_LOG11_SECTOR,		sizeof(nglog_data)},
// MH810100(S) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X (QR�m��E����f�[�^�Ή�)
//	{_TYP_DATA,		FLT_LOG12_SECTORMAX,		FLT_LOG12_SECTOR,		sizeof(IoLog_Data)},
	{_TYP_DATA,		FLT_LOG12_SECTORMAX,		FLT_LOG12_SECTOR,		sizeof(DC_QR_log)},
// MH810100(E) K.Onodera 2019/12/26 �Ԕԃ`�P�b�g���X (QR�m��E����f�[�^�Ή�)
	{_TYP_DATA,		FLT_LOG13_SECTORMAX,		FLT_LOG13_SECTOR,		sizeof(meisai_log)},
	{_TYP_DATA,		FLT_LOG14_SECTORMAX,		FLT_LOG14_SECTOR,		sizeof(meisai_log)},
	{_TYP_DATA,		FLT_LOG15_SECTORMAX,		FLT_LOG15_SECTOR,		sizeof(meisai_log)},
	{_TYP_DATA,		FLT_LOG16_SECTORMAX,		FLT_LOG16_SECTOR,		sizeof(remote_Change_data)},
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//	{_TYP_DATA,		FLT_LOG17_SECTORMAX,		FLT_LOG17_SECTOR,		sizeof(LongParking_data)},
	{_TYP_DATA,		FLT_LOG17_SECTORMAX,		FLT_LOG17_SECTOR,		sizeof(LongParking_data_Pweb)},//ParkingWeb�p
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	{_TYP_DATA,		FLT_LOG18_SECTORMAX,		FLT_LOG18_SECTOR,		sizeof(RismEvent_data)},
	{_TYP_DATA,		FLT_LOG18_SECTORMAX,		FLT_LOG18_SECTOR,		sizeof(DC_LANE_log)},
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
	{_TYP_DATA,		FLT_LOG19_SECTORMAX,		FLT_LOG19_SECTOR,		sizeof(SYUKEI)},
	{_TYP_DATA,		FLT_LOG20_SECTORMAX,		FLT_LOG20_SECTOR,		sizeof(Rmon_log)},
	{_TYP_SYSTEM,	2*FLT_PARAM_SECTORMAX,		FLT_PARAM_SECTOR_1,		FLT_PARAM_SECT_SIZE},
	{_TYP_DATA,		FLT_PRINTDATA_SECTORMAX,	FLT_PRINTDATA_SECTOR,	FLT_PRINTDATA_SECT_SIZE},
	{_TYP_DATA,		FLT_LOCKPARAM_SECTORMAX,	FLT_LOCKPARAM_SECTOR,	FLT_LOCKPARAM_SECT_SIZE},
	{_TYP_DATA,		FLT_CARDDATA_SECTORMAX,		FLT_CARDDATA_SECTOR,	FLT_CARDDATA_SECT_SIZE},
	{_TYP_DATA,		FLT_BRLOGSZ_SECTORMAX,		FLT_BRLOGSZ_SECTOR,		FLT_BRLOGSZ_SECT_SIZE},
	{_TYP_DATA,		FLT_BACKUP_SECTORMAX,		FLT_BACKUP_SECTOR0,		FLT_BACKUP_SECT_SIZE},
	{_TYP_SYSTEM,	FLT_SOUND0_SECTORMAX,		FLT_SOUND0_SECTOR,		FLT_SOUND0_SECT_SIZE},
	{_TYP_SYSTEM,	FLT_SOUND1_SECTORMAX,		FLT_SOUND1_SECTOR,		FLT_SOUND1_SECT_SIZE},
	{_TYP_SYSTEM,	FLT_PROGRAM_SECTORMAX,		FLT_PROGRAM_SECTOR,		FLT_PROGRAM_SECT_SIZE},
	{0,0,0,0}
};

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[API]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_init()
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V�����W���[������������
 *| param	: param_written - ��d���A�����Őݒ�p�����[�^�̏������݂��s��ꂽ/�s���Ȃ�	<OUT>
 *|						1 �� �s��ꂽ
 *|						0 �� �s���Ȃ�
 *| 		  param_ok - �ݒ�p�����[�^�̈�L��/����										<OUT>
 *|						1 �� �L��
 *|						0 �� ����
 *| 		  lockparam_ok - �Ԏ��p�����[�^�̈�L��/����									<OUT>
 *|						1 �� �L��
 *|						0 �� ����
 *|			  clr - 1=�o�b�N�A�b�vRAM�f�[�^�̋����N���A 0=�����N���A���Ȃ�(���d����������)
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_init(uchar *param_ok, uchar *lockparam_ok, ulong *ver_flg, char clr)
{
	uchar	i;
	ushort wk_BR_LOG_SIZE[eLOG_MAX];
	uchar	f_LogSzChg = 0;

	/* FROM2�h���C�o������ */
	Flash2Init();
	Flash1Init();

	// �p�����[�^�̈挈��
	if(BootInfo.sw.parm == OPERATE_AREA1){// ��1
		FLT_PARAM_SECTOR = FLT_PARAM_SECTOR_1;
	} else {
		FLT_PARAM_SECTOR = FLT_PARAM_SECTOR_2;
	}
	
	_flt_DisableTaskChg();
	
	if (!clr) {
	/* ��d���A���� */
		flt_Fukuden();
	}
	
	/* �p�����[�^���e�[�u�������� */
	AppServ_MakeParamInfoTable(&FLT_Ctrl_Backup.param_tbl);
	
	/* �o�b�N�A�b�v�G���A���e�[�u�������� */
	AppServ_MakeBackupDataInfoTable(&FLT_Ctrl_NotBackup.area_tbl);
	
	/* �t���b�V����̃f�[�^�`�F�b�N��LOG����f�[�^�̍쐬 */
	flt_ChkFlashData(&FLT_Ctrl_NotBackup.data_info, param_ok, lockparam_ok, ver_flg);
	
	/* �v���󂯕t���������� */
	for (i = 0; i < FLT_REQTBL_MAX; i++) {
		FLT_Ctrl_Backup.req_tbl[i].req		= FLT_REQ_IDLE;
		FLT_Ctrl_Backup.req_tbl[i].result	= _FLT_MkRtnVal2(FLT_NORMAL);
	}

	// �v���O�����o�[�W�����A�b�v�ɂāA�W�v(SYUKEI) or �ʐ��Z(Receipt_data)�̌`���ς�����ꍇ�̑΍�B
	// F1&F3 or F3&F5�N���i�������N���A�j���AFlashROM���̗��G���A�T�C�Y�����݂̃T�C�Y���قȂ�ꍇ�A
	// FlashROM���̏W�v/�ʐ��Z�f�[�^�������B
	// FlashROM���̗��G���A�T�C�Y�́A�o�b�N�A�b�v�f�[�^���� "BR_LOG_SIZE" ���瓾��B
	// �i�O��Ƃ��ăo�b�N�A�b�v�^���X�g�A�́u�o�b�N�A�b�v�v������ȑO�ɍs���Ă����K�v������B�j
	// �i�v���O�����o�[�W�����A�b�v���́A�o�b�N�A�b�v���s���菇�ƂȂ��Ă���j
	// �i���̎��_�ł̓^�X�N�y��WDT�͋N�����Ă��Ȃ��j

	if ( clr ) {	// F1&F3 or F3&F5
		for( i = 0; i < eLOG_MAX; i++ ){
			wk_BR_LOG_SIZE[i] = 0;
		}
		if( flt_CheckSum( FLT_BRLOGSZ_SECTOR, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE ) ){	// OK
			FLT_ReadBRLOGSZ( &wk_BR_LOG_SIZE[0] );							// �o�b�N�A�b�v���̃��O�T�C�Y����FlashROM����Ǐo��
		}
		for( i = 0; i < eLOG_MAX; i++ ){
			if( wk_BR_LOG_SIZE[i] != LogDatMax[i][0] ){						// �T�C�Y�ύX�`�F�b�N
				FLT_LogErase2(i);											// LOG��RAM�̈�ƃt���b�V���Ǘ��̈�N���A
				f_LogSzChg = 1;
			}
		}
		if( f_LogSzChg ){													// �T�C�Y�ύX����
			for( i = 0; i < eLOG_MAX ; i++ ){
				BR_LOG_SIZE[i] = LogDatMax[i][0];							// �V���O�T�C�Yset
			}
			FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );			// �V���O�T�C�Y��񏑂�����
		}
	}

	_flt_EnableTaskChg();
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteParam1()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^��������(�����Ƀo�C�i���C���[�W����)
 *| param	: flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteParam1(uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM1, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteParam1(&FLT_Ctrl_Backup.param_tbl, 
							&FLT_Ctrl_NotBackup.param_tbl_wk,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM1],
							FLT_CMP_CPRM_ONLY);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM1);
	}
	else {
		/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_PARAM1);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteParam2()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^��������(�����Ƀo�C�i���C���[�W�t��)
 *| param	: param_image - �p�����[�^�ۑ��C���[�W		<IN>
 *|			  image_size - param_image�̗L���f�[�^��
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteParam2(const char *param_image, size_t image_size, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.param2.param_image	= param_image;
	FLT_Ctrl_Backup.params.param2.image_size	= flt_CParamDataSize(&FLT_Ctrl_Backup.param_tbl);
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteParam2(&FLT_Ctrl_Backup.params.param2, 
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM2]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_PARAM2);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG��������
 *| param	: id - ���O���
 *|			  records - ���ZLOG���R�[�h�擪		<IN>
 *|			  record_num - records���̐��ZLOG���R�[�h����(1�`FLT_SALELOG_RECORDNUM_MAX)
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *|			  kind - 4Kbyte2�ʂ̂ǂ���̏�������������
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong FLT_WriteLog(short id, const char *records, short record_num, uchar flags, short kind)
{
	ulong	ret;
	uchar	reqid = (uchar)(FLT_REQTBL_LOG+id);

	if( record_num==0 ){		// ����Ƃ���
		return _FLT_MkRtnVal2(FLT_NORMAL);
	}
	if( record_num>LOG_SECORNUM(id) ){
	// ���������܂ŏ�������
	// �����d�l��AFLT_LOG_RECORDNUM_MAX=LOG_CNT�Ȃ̂ł��肦�Ȃ�
		record_num = LOG_SECORNUM(id);
	}

	/* �����Ώۗ̈�̃��b�N */
	if( !flt_LockFlash(FLT_Ctrl_Backup.req_tbl, reqid, FLT_REQ_WRITE, flags) ){
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.log[id].records = records;
	FLT_Ctrl_Backup.params.log[id].record_num = record_num;
	FLT_Ctrl_Backup.params.log[id].sramlog_kind = kind;		// SRAM���LOG(4Kbyte2�ʂ̂ǂ���̏��������������������f�[�^)

	memcpy(FLT_Ctrl_Backup.data_info_bk.log_mgr+id,
			FLT_Ctrl_NotBackup.data_info.log_mgr+id,
								sizeof(t_FltLogMgr));		/* ���d�����p�Ƀo�b�N�A�b�v */
	FLT_Ctrl_Backup.req_tbl[reqid].req = FLT_REQ_EXEC_WRITE;	/* ��d�ۏ؂̃`�F�b�N�|�C���g */

	if( flags&FLT_EXCLUSIVE ){	// �t���b�V���^�X�N���g��Ȃ�
		_flt_DisableTaskChg();
		ret = flt_WriteLog(FLT_Ctrl_Backup.params.log+id, 
						FLT_Ctrl_NotBackup.data_info.log_mgr+id,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG],
							id);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, reqid);
	}
	else {		// �������݊����͊O�ő҂�
		ret = _FLT_MkRtnVal(FLT_NORMAL, reqid);
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLogo()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g���S��������
 *| param	: image - �v�����g���S�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteLogo(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOGO, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.logo.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteLogo(&FLT_Ctrl_Backup.params.logo,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOGO);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_LOGO);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteHeader()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�w�b�_��������
 *| param	: image - �v�����g�w�b�_�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteHeader(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_HEADER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.header.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteHeader(&FLT_Ctrl_Backup.params.header,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_HEADER]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_HEADER);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_HEADER);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�t�b�^��������
 *| param	: image - �v�����g�t�b�^�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteFooter(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FOOTER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.footer.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteFooter(&FLT_Ctrl_Backup.params.footer,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FOOTER]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FOOTER);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_FOOTER);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteAcceptFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��t���t�b�^��������
 *| param	: image - �v�����g�t�b�^�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteAcceptFooter(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_ACCEPTFOOTER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.acceptfooter.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteAcceptFooter(&FLT_Ctrl_Backup.params.acceptfooter,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ACCEPTFOOTER]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_ACCEPTFOOTER);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_ACCEPTFOOTER);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLockParam1()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��p�����[�^��������(�����Ƀo�C�i���C���[�W����)
 *| param	: flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteLockParam1(uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM1, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteLockParam1(FLT_Ctrl_Backup.workbuf, &FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM1]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM1);
	}
	else {
		/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_LOCKPARAM1);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLockParam2()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��p�����[�^��������(�����Ƀo�C�i���C���[�W�t��)
 *| param	: param_image - �p�����[�^�ۑ��C���[�W		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteLockParam2(const char *param_image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM2, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.lockparam2.param_image	= param_image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteLockParam2(&FLT_Ctrl_Backup.params.lockparam2, 
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM2);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_LOCKPARAM2);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_Backup()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v
 *| param	: sale_records - ���ZLOG���R�[�h
 *|			  sale_record_num - ���ZLOG���R�[�h����
 *|			  total_records - �W�vLOG���R�[�h
 *|			  total_record_num - �W�vLOG���R�[�h����
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_Backup(ulong ver_flg,uchar flags)
{
	ulong ret;

	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BACKUP, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.bkup.ver_flg = ver_flg;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ� */
		_flt_DisableTaskChg();
		ret = flt_Backup(
						&FLT_Ctrl_Backup.params.bkup,
						&FLT_Ctrl_Backup.param_tbl,
						&FLT_Ctrl_NotBackup.param_tbl_wk,
						&FLT_Ctrl_NotBackup.area_tbl,
						&FLT_Ctrl_NotBackup.area_tbl_wk,
						FLT_Ctrl_Backup.workbuf);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BACKUP);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_BACKUP);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_RestoreParam()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^���X�g�A
 *| param	: none
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_RestoreParam(void)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2, FLT_REQ_READ, FLT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	
	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_RestoreParam(&FLT_Ctrl_Backup.param_tbl, &FLT_Ctrl_NotBackup.param_tbl_wk);
	_flt_EnableTaskChg();
	
	/* �����Ώۗ̈�̃��b�N���� */
	flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2);
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_RestoreLockParam()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��p�����[�^���X�g�A
 *| param	: none
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_RestoreLockParam(void)
{
	ulong ret;
	
	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_RestoreLockParam();
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetSaleLogRecordNum()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ZLOG���R�[�h�����擾
 *| param	: record_num - LOG���R�[�h����						<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_GetSaleLogRecordNum(long *record_num)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetTotalLogRecordNum()
 *[]----------------------------------------------------------------------[]
 *| summary	: �W�vLOG���R�[�h�����擾
 *| param	: record_num - LOG���R�[�h����						<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_GetTotalLogRecordNum(long *record_num)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindFirstSaleLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ZLOG�擾(�擪�̃��R�[�h)
 *| param	: handle - �����n���h��						<OUT>
 *|			  buf - �ŌÂ̐��ZLOG�f�[�^1�����̃f�[�^	<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindFirstSaleLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextSaleLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ZLOG�擾(2���ڈȍ~�̃��R�[�h)
 *| param	: handle - �����n���h��						<IN/OUT>
 *|			  buf - ���ZLOG2���ڈȍ~�̃f�[�^			<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextSaleLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextSaleLog_OnlyDate()	�����������p
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ZLOG�̓o�^���������擾(2���ڈȍ~�̃��R�[�h)
 *| param	: handle - �����n���h��						<IN/OUT>
 *|			  buf - ���ZLOG2���ڈȍ~�̃f�[�^			<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextSaleLog_OnlyDate(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindFirstSaleLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: �擪�i�ŌÁj���炎�Ԗڂ̃f�[�^�Ǐo��
 *| param	: num - 0�`719 (0=�Ō�)
 *|			  buf - ���ZLOG�f�[�^1�����̃f�[�^	<OUT>
 *|					sizeof(Receipt_data)
 *|			  ���O��FlashROM���̓o�^�������擾���A�L���f�[�^�݂̂̓Ǐo����
 *|			  �s�����Ƃ���{�Ƃ���B
 *| return	: 0x00XXXXXX - ����I��
 *|			  0x05XXXXXX - FLT_BUSY
 *|			  0x06XXXXXX - FLT_NODATA
 *[]----------------------------------------------------------------------[]
 *|	Auther	: Okuda : 2005/08/18
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_TargetSaleLogRead( ushort num, char *buf )
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindFirstTotalLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: �W�vLOG�擾(�擪�̃��R�[�h)
 *| param	: handle - �����n���h��						<OUT>
 *|			  buf - �ŌÂ̏W�vLOG�f�[�^1�����̃f�[�^	<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindFirstTotalLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextTotalLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ZLOG�擾(2���ڈȍ~�̃��R�[�h)
 *| param	: handle - �����n���h��						<IN/OUT>
 *|			  buf - ���ZLOG2���ڈȍ~�̃f�[�^			<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextTotalLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextTotalLog_OnlyDate()	�����������p
 *[]----------------------------------------------------------------------[]
 *| summary	: �W�vLOG�̓o�^���������擾(2���ڈȍ~�̃��R�[�h)
 *| param	: handle - �����n���h��						<IN/OUT>
 *|			  buf - ���ZLOG2���ڈȍ~�̃f�[�^			<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextTotalLog_OnlyDate(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_TargetTotalLogRead()
 *[]----------------------------------------------------------------------[]
 *| summary	: �擪�i�ŌÁj���炎�Ԗڂ̃f�[�^�Ǐo��
 *| param	: num - 0�`20 (0=�Ō�)
 *|			  buf - ���ZLOG�f�[�^1�����̃f�[�^	<OUT>
 *|					sizeof(Syu_log)
 *|			  ���O��FlashROM���̓o�^�������擾���A�L���f�[�^�݂̂̓Ǐo����
 *|			  �s�����Ƃ���{�Ƃ���B
 *| return	: 0x00XXXXXX - ����I��
 *|			  0x05XXXXXX - FLT_BUSY
 *|			  0x06XXXXXX - FLT_NODATA
 *[]----------------------------------------------------------------------[]
 *|	Auther	: Okuda : 2005/08/19
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_TargetTotalLogRead( ushort num, char *buf )
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief		LOG�擾(�擪�̃��R�[�h)
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG���
///	@param[in/out]	handle	: �����n���h��
///	@param[out]	buf			: �ŌÂ�LOG1�����̃f�[�^
///	@return					: 0x00XXXXXX - ����I��
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_FindFirstLog(short id, t_FltLogHandle *handle, char *buf)
{
	ulong ret;

	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, (uchar)(FLT_REQTBL_LOG+id), FLT_REQ_READ, FLT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}

	/* �����n���h�������� */
	handle->kind		= FLT_SEARCH_LOG + id;
	handle->sectno		= FLT_Ctrl_NotBackup.data_info.log_mgr[id].secttop;
	handle->recordno	= 0;
	/* �{�������s */
	ret = flt_ReadLogRecord(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle, buf);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		FLT_CloseLogHandle(handle);
		return ret;
	}
	/* �����n���h���X�V */
	flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle);

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�擾(2���ڈȍ~�̃��R�[�h)
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG���
///	@param[in/out]	handle	: �����n���h��
///	@param[out]	buf			: LOG2���ڈȍ~�̃f�[�^
///	@return					: 0x00XXXXXX - ����I��
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_FindNextLog(short id, t_FltLogHandle *handle, char *buf)
{
	ulong ret;

	if (handle->kind != FLT_SEARCH_LOG+id) {
		return _FLT_MkRtnVal2(FLT_PARAM_ERR);
	}

	/* �����Ώۗ̈悪���b�N����Ă��邩�`�F�b�N */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG + id].req != FLT_REQ_READ) {
		return _FLT_MkRtnVal2(FLT_NOT_LOCKED);	/* ���b�N����Ă��Ȃ� */
	}

	/* �{�������s */
	ret = flt_ReadLogRecord(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle, buf);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* �����n���h���X�V */
		flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�̓o�^���������擾(2���ڈȍ~�̃��R�[�h)
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG���
///	@param[in/out]	handle	: �����n���h��
///	@param[out]	buf			: LOG2���ڈȍ~�̃f�[�^
///	@return					: 0x00XXXXXX - ����I��
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_FindNextLog_OnlyDate(short id, t_FltLogHandle *handle, char *buf)
{
	ulong ret;
	t_FltLogMgr *logmgr;
	ulong src_addr;

	/* �{�������s */
	logmgr = FLT_Ctrl_NotBackup.data_info.log_mgr+id;
	if (logmgr->sectmax <= handle->sectno) {	// �C�j�V�����R�[�f�B���O
		ret = _FLT_MkRtnVal2(FLT_NODATA);	/* �w��ʒu�Ƀ��R�[�h���� */
	}
	else if (handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		ret = _FLT_MkRtnVal2(FLT_NODATA);	/* �w��ʒu�Ƀ��R�[�h���� */
	}
	else {
		/* �f�[�^�ǂݏo�� */
		src_addr = logmgr->sectinfo[handle->sectno].address 
				+ FLT_LOGOFS_RECORD
				+ (handle->recordno * logmgr->record_size);
		Flash2Read((uchar*)buf, src_addr, (ulong)25L);	// �o�^�����܂�get
		ret = _FLT_MkRtnVal2(FLT_NORMAL);
	}

	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* �����n���h���X�V */
		flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		�擪�i�ŌÁj���炎�Ԗڂ̃f�[�^�Ǐo��
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG���
///	@param[in]	num			: 0�`20 (0=�Ō�)
///	@param[out]	buf			: LOG1�����̃f�[�^
///	@return					: 0x00XXXXXX - ����I��
///							  0x05XXXXXX - FLT_BUSY
///							  0x06XXXXXX - FLT_NODATA
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_TargetLogRead( short id, ushort num, char *buf )
{
	ulong 	ulret;
	ushort	i;
	t_FltLogHandle handle;
	long	recn;
	uchar	reqid = (uchar)(FLT_REQTBL_LOG+id);

	FLT_GetLogRecordNum(id, &recn);
	if( num>=recn ){		// ����Ƃ���
		return _FLT_MkRtnVal2(FLT_NORMAL);
	}

	/* �����Ώۗ̈�̃��b�N */
	if( !flt_LockFlash(FLT_Ctrl_Backup.req_tbl, reqid, FLT_REQ_READ, FLT_EXCLUSIVE) ){
		return _FLT_MkRtnVal2(FLT_BUSY);		/* ���b�N�ł��Ȃ����� */
	}

	/* �����n���h�������� */
	handle.kind		= FLT_SEARCH_LOG+id;
	handle.sectno	= FLT_Ctrl_NotBackup.data_info.log_mgr[id].secttop;
	handle.recordno	= 0;

	/* �{�������s */
	/* Target�ʒu�܂�Skip */
	for(i=0; i<num; i++){
		ulret = flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, &handle);
		if(_FLT_RtnKind(ulret)!=FLT_NORMAL ){	/* error (Y) */
			FLT_CloseLogHandle(&handle);		/* Handle & Lock release */
			return	ulret;						/* error return */
		}
		WACDOG;
	}

	/* Terget Log�f�[�^�Ǐo�� */
	ulret = flt_ReadLogRecord(FLT_Ctrl_NotBackup.data_info.log_mgr+id, &handle, buf);

	FLT_CloseLogHandle( &handle );				/* Handle & Lock release */

	return ulret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG���R�[�h�����擾
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG���
///	@param[out]	record_num	: LOG���R�[�h����
///	@return					: 0x00XXXXXX - ����I��
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_GetLogRecordNum(short id, long *record_num)
{
	/* �����Ώۗ̈�̃��b�N */
	if( !flt_LockFlash(FLT_Ctrl_Backup.req_tbl, (uchar)(FLT_REQTBL_LOG+id), FLT_REQ_READ, FLT_EXCLUSIVE) ){
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}

	/* �{�������s */
	*record_num = flt_GetLogRecordNum(FLT_Ctrl_NotBackup.data_info.log_mgr+id);

	/* �����Ώۗ̈�̃��b�N���� */
	flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, (uchar)(FLT_REQTBL_LOG+id));

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadLogo()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g���S�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadLogo(char *buf)
{
	ulong ret;

	/* �{�������s */
	ret = flt_ReadLogo(buf, (char)0, 0, 0);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadLogoPart()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g���S�擾�i�f�[�^�����Ǐo���j
 *|			  �w��I�t�Z�b�g�ʒu(TopOffset)���炵�Ă��T�C�Y��(ReadSize)
 *|			  �Ǐo�����s�� *buf �ɃZ�b�g����B
 *| param	: buf 							<OUT>
 *| 		  TopOffset						<IN>
 *| 		  ReadSize						<IN>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadLogoPart(char *buf, ushort TopOffset, ushort ReadSize)
{
	ulong ret;

	/* �{�������s */
	ret = flt_ReadLogo(buf, (char)1, TopOffset, ReadSize);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadHeader()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�w�b�_�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadHeader(char *buf)
{
	ulong ret;

	/* �{�������s */
	ret = flt_ReadHeader(buf);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadFooter(char *buf)
{
	ulong ret;

	/* �{�������s */
	ret = flt_ReadFooter(buf);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadAcceptFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��t���t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadAcceptFooter(char *buf)
{
	ulong ret;

	/* �{�������s */
	ret = flt_ReadAcceptFooter(buf);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_Restore()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���X�g�A���s
 *| param	: flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_Restore(uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_RESTORE, FLT_REQ_READ, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��Ń��X�g�A*/
		_flt_DisableTaskChg();
		ret = flt_Restore(&FLT_Ctrl_Backup.param_tbl,
						&FLT_Ctrl_NotBackup.param_tbl_wk,
						&FLT_Ctrl_NotBackup.area_tbl,
						&FLT_Ctrl_NotBackup.area_tbl_wk);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_RESTORE);
	}
	else {
	/* ���X�g�A�����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_RESTORE);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ChkWriteCmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: �������݊����`�F�b�N
 *| param	: req_kind - �������`�F�b�N���鏑�����ݎ��(��������API�̖߂�l)
 *|			  endsts - �������݌���			<OUT>
 *|					0x00XXXXXX - ����I��
 *| return	: 1 - ����
 *|			  0 - ������
 *[]----------------------------------------------------------------------[]*/
uchar	FLT_ChkWriteCmp(ulong req_kind, ulong *endsts)
{
	if (req_kind >= FLT_REQTBL_MAX) {
		*endsts = _FLT_MkRtnVal2(FLT_PARAM_ERR);
		return 1;
	}
	if (FLT_Ctrl_Backup.req_tbl[req_kind].req == FLT_REQ_IDLE) {
		*endsts = FLT_Ctrl_Backup.req_tbl[req_kind].result;
		return 1;
	}
	
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_CloseLogHandle()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG�����n���h���N���[�Y
 *| param	: handle - LOG�����n���h��					<IN>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	FLT_CloseLogHandle(t_FltLogHandle *handle)
{
	// �����Ώۗ̈�̃��b�N����
	if( handle->kind<=FLT_SEARCH_LOG+LOG_STRAGEN ){
		if( FLT_Ctrl_Backup.req_tbl[handle->kind].req==FLT_REQ_READ ){
			flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, handle->kind);
		}
	}
	// �n���h��������
	handle->kind = FLT_SEARCH_NONE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetProgramVersion()
 *[]----------------------------------------------------------------------[]
 *| summary	: FLASH��ɕۑ����ꂽ�v���O�����o�[�W�������擾����
 *| param	: ver_flg - �擾�����o�[�W�����A�b�v�t���O			<OUT>
 *| return	: 0 - ���X�g�A�p�̃f�[�^�Ȃ�
 *[]----------------------------------------------------------------------[]*/
uchar	FLT_GetVerupFlag(ulong *ver_flg)
{
	ulong dummy;
	
	return flt_GetVersion(&dummy, ver_flg);
}


/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetParamTable()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^���e�[�u���擾
 *| param	: none
 *| return	: �p�����[�^���e�[�u��
 *[]----------------------------------------------------------------------[]*/
const t_AppServ_ParamInfoTbl	*FLT_GetParamTable(void)
{
	return &FLT_Ctrl_Backup.param_tbl;
}

/*====================================================================================[TASK]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: flatask()
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V���^�X�N�{��
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flatask(void)
{
	int	i;
	ulong	ret;
	uchar	exec;
	t_FltReq *req_tbl = FLT_Ctrl_Backup.req_tbl;
	
	FLT_f_TaskStart = 1;						/* FlashROM�^�X�N�N�������Z�b�g */

	while (1) {
		taskchg(IDLETSKNO);
		
		exec = FLT_REQTBL_MAX;
		
		if (req_tbl[FLT_REQTBL_BACKUP].req == FLT_REQ_WRITE) {
		/* �o�b�N�A�b�v */
			ret = flt_Backup(
							&FLT_Ctrl_Backup.params.bkup,
							&FLT_Ctrl_Backup.param_tbl,
							&FLT_Ctrl_NotBackup.param_tbl_wk,
							&FLT_Ctrl_NotBackup.area_tbl,
							&FLT_Ctrl_NotBackup.area_tbl_wk,
							FLT_Ctrl_Backup.workbuf);
			exec = FLT_REQTBL_BACKUP;
		}
		else if (req_tbl[FLT_REQTBL_RESTORE].req == FLT_REQ_READ) {
		/* ���X�g�A */
			ret = flt_Restore(&FLT_Ctrl_Backup.param_tbl,
							&FLT_Ctrl_NotBackup.param_tbl_wk,
							&FLT_Ctrl_NotBackup.area_tbl,
							&FLT_Ctrl_NotBackup.area_tbl_wk);
			exec = FLT_REQTBL_RESTORE;
		}
		else if (req_tbl[FLT_REQTBL_PARAM2].req == FLT_REQ_WRITE) {
		/* �p�����[�^(�C���[�W�쐬�ς�) */
			ret = flt_WriteParam2(&FLT_Ctrl_Backup.params.param2,
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf, 
							&req_tbl[FLT_REQTBL_PARAM2]);
			exec = FLT_REQTBL_PARAM2;
		}
		else if (req_tbl[FLT_REQTBL_PARAM1].req == FLT_REQ_WRITE) {
		/* �p�����[�^(�C���[�W����) */
			ret = flt_WriteParam1(&FLT_Ctrl_Backup.param_tbl, 
							&FLT_Ctrl_NotBackup.param_tbl_wk,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM1],
							FLT_CMP_CPRM_ONLY);
			exec = FLT_REQTBL_PARAM1;
		}
		else if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2].req == FLT_REQ_WRITE) {
		/* �Ԏ��p�����[�^(�C���[�W�t��) */
			ret = flt_WriteLockParam2(&FLT_Ctrl_Backup.params.lockparam2,
							FLT_Ctrl_Backup.workbuf, 
							&req_tbl[FLT_REQTBL_LOCKPARAM2]);
			exec = FLT_REQTBL_LOCKPARAM2;
		}
		else if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM1].req == FLT_REQ_WRITE) {
		/* �Ԏ��p�����[�^(�C���[�W����) */
			ret = flt_WriteLockParam1(FLT_Ctrl_Backup.workbuf, &FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM1]);
			exec = FLT_REQTBL_LOCKPARAM1;
		}
		else if (req_tbl[FLT_REQTBL_LOGO].req == FLT_REQ_WRITE) {
		/* ���S */
			ret = flt_WriteLogo(&FLT_Ctrl_Backup.params.logo, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_LOGO]);
			exec = FLT_REQTBL_LOGO;
		}
		else if (req_tbl[FLT_REQTBL_HEADER].req == FLT_REQ_WRITE) {
		/* �w�b�_ */
			ret = flt_WriteHeader(&FLT_Ctrl_Backup.params.header, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_HEADER]);
			exec = FLT_REQTBL_HEADER;
		}
		else if (req_tbl[FLT_REQTBL_FOOTER].req == FLT_REQ_WRITE) {
		/* �t�b�^ */
			ret = flt_WriteFooter(&FLT_Ctrl_Backup.params.footer, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_FOOTER]);
			exec = FLT_REQTBL_FOOTER;
		}
		else if (req_tbl[FLT_REQTBL_ACCEPTFOOTER].req == FLT_REQ_WRITE) {
		/* ��t���t�b�^ */
			ret = flt_WriteAcceptFooter(&FLT_Ctrl_Backup.params.acceptfooter, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_ACCEPTFOOTER]);
			exec = FLT_REQTBL_ACCEPTFOOTER;
		}
		else if (req_tbl[FLT_REQTBL_SYOMEI].req == FLT_REQ_WRITE) {
		/* ������ */
			ret = flt_WriteSyomei(&FLT_Ctrl_Backup.params.syomei, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_SYOMEI]);
			exec = FLT_REQTBL_SYOMEI;
		}

		else if (req_tbl[FLT_REQTBL_KAMEI].req == FLT_REQ_WRITE) {
		/* �����X */
			ret = flt_WriteKamei(&FLT_Ctrl_Backup.params.kamei, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_KAMEI]);
			exec = FLT_REQTBL_KAMEI;
		}
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		else if (req_tbl[FLT_REQTBL_EDYAT].req == FLT_REQ_WRITE) {
//		/* �d�����`�s�R�}���h */
			ret = flt_WriteEdyAt(&FLT_Ctrl_Backup.params.edyat, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_EDYAT]);
			exec = FLT_REQTBL_EDYAT;
		}
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#endif
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
		else if (req_tbl[FLT_REQTBL_USERDEF].req == FLT_REQ_WRITE) {
		/* ���[�U��` */
			ret = flt_WriteUserDef(&FLT_Ctrl_Backup.params.userdef, FLT_Ctrl_Backup.workbuf,
										&req_tbl[FLT_REQTBL_USERDEF]);
			exec = FLT_REQTBL_USERDEF;
		}
		else if (req_tbl[FLT_REQTBL_ETC].req == FLT_REQ_WRITE) {
		/* ETC cache */
			ret = flt_WriteEtc(&FLT_Ctrl_Backup.params.etc, &req_tbl[FLT_REQTBL_ETC]);
			exec = FLT_REQTBL_ETC;
		}
		else if (req_tbl[FLT_REQTBL_IMAGE].req == FLT_REQ_WRITE) {
		/* program image */
			ret = flt_WriteImage(&FLT_Ctrl_Backup.params.image, &req_tbl[FLT_REQTBL_IMAGE]);
			exec = FLT_REQTBL_IMAGE;
		}
		else if (req_tbl[FLT_REQTBL_PARAM3].req == FLT_REQ_WRITE) {
		/* �ҋ@�ʃp�����[�^(�C���[�W�쐬�ς�) */
			ret = flt_WriteParam3(&FLT_Ctrl_Backup.params.param3,
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf, 
							&req_tbl[FLT_REQTBL_PARAM3]);
			exec = FLT_REQTBL_PARAM3;
		}
		else if (req_tbl[FLT_REQTBL_AZUFTR].req == FLT_REQ_WRITE) {
			/* �a��؃t�b�^ */
			ret = flt_WriteAzuFtr(&FLT_Ctrl_Backup.params.azuftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_AZUFTR]);
			exec = FLT_REQTBL_AZUFTR;
		}
// �d�l�ύX(S) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
		else if (req_tbl[FLT_REQTBL_CREKBRFTR].req == FLT_REQ_WRITE) {
			/* �N���W�b�g�ߕ����t�b�^ */
			ret = flt_WriteCreKbrFtr(&FLT_Ctrl_Backup.params.crekbrftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_CREKBRFTR]);
			exec = FLT_REQTBL_CREKBRFTR;
		}
		else if (req_tbl[FLT_REQTBL_EPAYKBRFTR].req == FLT_REQ_WRITE) {
			/* �d�q�}�l�[�ߕ����t�b�^ */
			ret = flt_WriteEpayKbrFtr(&FLT_Ctrl_Backup.params.epaykbrftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_EPAYKBRFTR]);
			exec = FLT_REQTBL_EPAYKBRFTR;
		}
// �d�l�ύX(E) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
		else if (req_tbl[FLT_REQTBL_FUTUREFTR].req == FLT_REQ_WRITE) {
			/* ����x���z�t�b�^ */
			ret = flt_WriteFutureFtr(&FLT_Ctrl_Backup.params.futureftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_FUTUREFTR]);
			exec = FLT_REQTBL_FUTUREFTR;
		}
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		else if (req_tbl[FLT_REQTBL_EMGFOOTER].req == FLT_REQ_WRITE) {
		/* ��Q�A���[�t�b�^ */
			ret = flt_WriteEmgFooter(&FLT_Ctrl_Backup.params.emgfooter, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_EMGFOOTER]);
			exec = FLT_REQTBL_EMGFOOTER;
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		else{
			for(i=FLT_REQTBL_LOG; i<FLT_REQTBL_PARAM1; i++){	// �����f�[�^
				if( req_tbl[i].req==FLT_REQ_EXEC_WRITE ){
					ret = flt_WriteLog(FLT_Ctrl_Backup.params.log+i, 
							FLT_Ctrl_NotBackup.data_info.log_mgr+i,
							&FLT_Ctrl_Backup.req_tbl[i],
							i);
					exec = i;
					break;
				}
			}
		}

		if (exec != FLT_REQTBL_MAX) {
		/* �����ꂩ�̏������s��ꂽ */
			/* ���s���ʕۑ� */
			req_tbl[exec].result = ret;
			/* �̈惍�b�N���� */
			flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, exec);
		}
	}
}

/*====================================================================================[OTHERS]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Fukuden()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���d����
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_Fukuden(void)
{
// ���O�̕��d�͂�����ȑO��FLT_WriteLog_Pon()�Ŏ��{�ς�
	ulong	ret;

/* �v�����g���S */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO].req = FLT_REQ_IDLE;
	}
/* �v�����g�w�b�_ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_HEADER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_HEADER].req = FLT_REQ_IDLE;
	}
/* �v�����g�t�b�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FOOTER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FOOTER].req = FLT_REQ_IDLE;
	}
/* ��t���t�b�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ACCEPTFOOTER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ACCEPTFOOTER].req = FLT_REQ_IDLE;
	}
/* �v�����g������ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_SYOMEI].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_SYOMEI].req = FLT_REQ_IDLE;
	}
/* �v�����g�����X */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_KAMEI].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_KAMEI].req = FLT_REQ_IDLE;
	}
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EDYAT].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EDYAT].req = FLT_REQ_IDLE;
	}
// ���[�U��`
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_USERDEF].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_USERDEF].req = FLT_REQ_IDLE;
	}
// ETC cache
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ETC].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector((uchar*)FLT_Ctrl_Backup.params.etc.image);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ETC].req = FLT_REQ_IDLE;
	}
	/* �v���O�����C���[�W�͕��d�������s��Ȃ� */
	/* �ҋ@�ʃp�����[�^�C���[�W�͕��d�������s��Ȃ� */

/* �ݒ�p�����[�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM2].req == FLT_REQ_EXEC_WRITE) {
		ret = flt_WriteParamSector(FLT_PARAM_SECTOR, FLT_Ctrl_Backup.workbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			_flt_DestroyParamRamSum();		/* RAM��̃p�����[�^�T�����N���A */
		}
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM2].req = FLT_REQ_IDLE;
	}
/* �Ԏ��p�����[�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2].req == FLT_REQ_EXEC_WRITE) {
		ret = flt_WriteParamSector(FLT_LOCKPARAM_SECTOR, FLT_Ctrl_Backup.workbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			_flt_DestroyLockParamRamSum();		/* RAM��̃p�����[�^�T�����N���A */
		}
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2].req = FLT_REQ_IDLE;
	}
/* T�J�[�h�t�b�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_TCARDFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_TCARDFTR].req = FLT_REQ_IDLE;
	}
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CARDDATA].req == FLT_REQ_EXEC_WRITE) {
		flt_WriteSector( FLT_CARDDATA_SECTOR, FLT_Ctrl_Backup.workbuf, FLT_CARDDATA_SECT_SIZE);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CARDDATA].req = FLT_REQ_IDLE;
	}
/* �a��؃t�b�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_AZUFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_AZUFTR].req = FLT_REQ_IDLE;
	}
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
/* �N���W�b�g�ߕ����t�b�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CREKBRFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CREKBRFTR].req = FLT_REQ_IDLE;
	}
/* �d�q�}�l�[�ߕ����t�b�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EPAYKBRFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EPAYKBRFTR].req = FLT_REQ_IDLE;
	}
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
/* ����x���z�t�b�^ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FUTUREFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FUTUREFTR].req = FLT_REQ_IDLE;
	}
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EMGFOOTER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EMGFOOTER].req = FLT_REQ_IDLE;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ChkFlashData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �e�̈�̃T�����`�F�b�N���A�f�[�^���L�������𔻒f����
 *|			  �T��NG�Ńu�����N�łȂ��ꍇ�́A�C���[�X���s��
 *| param	: data_info - �e�̈�̊Ǘ��f�[�^			<OUT>
 *|			  param_ok - �ݒ�p�����[�^�̈�̗L��/����	<OUT>
 *|						1 �� �L��
 *|						0 �� ����
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_ChkFlashData(t_FltDataInfo *data_info, 
						uchar *param_ok, 
						uchar *lockparam_ok, 
						ulong *ver_flg)
{
	uchar restore_ok;
	
/* LOG�̈� */
	if( read_rotsw() == 4 ){	// ----- �X�[�p�[�C�j�V�����C�Y(RSW=4)? -----
		flt_ChkLog(data_info->log_mgr);
	}
	
/* �o�b�N�A�b�v�̈� */
	restore_ok = flt_CheckAreaSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE, 1);// �e�[�u���̃`�F�b�N
/* ���ʃp�����[�^�A�ʃp�����[�^�̈� */
	*param_ok = flt_CheckAreaSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
// �ҋ@�ʂɂ��Ă��`�F�b�N
	if(BootInfo.sw.parm == OPERATE_AREA1) {
		flt_CheckAreaSum(FLT_PARAM_SECTOR_2, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
	} else {
		flt_CheckAreaSum(FLT_PARAM_SECTOR_1, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
	}
	restore_ok &= *param_ok;
/* �Ԏ��p�����[�^�̈� */
	*lockparam_ok = flt_CheckAreaSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE, FLT_LOCKPARAM_SECTORMAX);
	restore_ok &= *lockparam_ok;
/* �v�����^���S�A�w�b�_�A�t�b�^�̈� */
	flt_ChkPrintData(FLT_Ctrl_Backup.workbuf, &FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO]);
	
	if (restore_ok) {
/* �o�[�W�����A�b�v�t���O�擾 */
		Flash2Read((uchar*)ver_flg, 
// ���s�́u2�v��t_AppServ_AreaInfoTbl->area_num(ushort)
				FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE+2+sizeof(t_AppServ_AreaInfo)*FLT_DTNUM_VERUPFLG, 
				sizeof(*ver_flg));
	}
	else {
		*ver_flg = 0;	/* �o�[�W�����A�b�v�v���Ȃ� */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ChkLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V����̐��Z�E�W�vLOG�f�[�^�ƁA���̊Ǘ��f�[�^�̏�����
 *| param	: salelog_mgr - ���ZLOG�Ǘ��f�[�^
 *| 		  totallog_mgr - �W�vLOG�Ǘ��f�[�^
 *| return	: none
 // iiduka 2012/02/22
 *[]----------------------------------------------------------------------[]*/
//	�w�胍�O�̃N���A�����{���邽�߂̊֐��ɕύX���ꂽ�B
//	�]���@��ł̓��O�G���A�̔�o�b�N�A�b�v�Ǘ��f�[�^�����������邽�߂̊֐����������A
//	���̃G���A���o�b�N�A�b�v�����悤�ɕύX����Ă���B
void	flt_ChkLog(t_FltLogMgr *log_mgr)
{
	short	i, j;

	// LOG������
	for(i=0; i<LOG_STRAGEN; i++){
		memset(log_mgr, 0, sizeof(t_FltLogMgr));
		log_mgr->sectmax = from2_map[i].num;
		log_mgr->record_size = from2_map[i].size;
		log_mgr->recordnum_max = LOG_SECORNUM(i);
		for( j = 0; j < log_mgr->sectmax; j++ ){
			log_mgr->sectinfo[j].address = j*(LogDatMax[i][1]+FLT_LOGOFS_RECORD) + from2_map[i].top;
		}
		log_mgr++;
	}
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ChkLog_all()
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V��+RAM��LOG�̊Ǘ��̈�̏�����
 *| param	: kind  eLog���
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	FLT_ChkLog_all( void )
{
	int i;
	for(i = 0; i < eLOG_MAX ; i++){
		memset( &LOG_DAT[i], 0, sizeof(struct log_record) );
	}
	flt_ChkLog(&FLT_Ctrl_NotBackup.data_info.log_mgr[0]);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CheckAreaSum()
 *[]----------------------------------------------------------------------[]
 *| summary	: FLASH��̈�̃`�F�b�N�T���ƍ�
 *| param	: top_address - �̈�擪�A�h���X
 *|			  sectsize - �Z�N�^�T�C�Y
 *|			  sectnum - �Z�N�^��
 *| return	: TRUE - �T��OK
 *|-----------------------------------------------------------------------[]
 *| remark	: �T��NG�̗̈悪�u�����N�łȂ�������A�C���[�X���s��
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckAreaSum(ulong top_address, ulong sectsize, ushort sectnum)
{
	BOOL	sum_ok, blank;
	
	sum_ok = flt_CheckData(top_address, sectsize*sectnum, &blank);
	if (!sum_ok && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		flt_EraseSectors(top_address, sectsize, (ulong)sectnum);	/* �T�����֌W����Z�N�^���C���[�X */
	}
	
	return sum_ok;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ChkPrintData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�f�[�^�̈�̊e�f�[�^�L�����`�F�b�N����
 *|			  �T��NG�Ńu�����N�łȂ��ꍇ�́A���̃f�[�^�̃C���[�X���s��
 *| param	: wbuf - �������ݗp���[�N�o�b�t�@			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_ChkPrintData(uchar *wbuf, t_FltReq *req_tbl)
{
	BOOL	sum, blank, write;
	
	write = FALSE;
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, _ETC_SIZE_);
	
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_LOGO, 
					FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_LOGO], 0xFF, FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE);			/* �f�[�^�C���[�X */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_HEADER, 
					FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_HEADER], 0xFF, FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FOOTER, 
					FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_FOOTER], 0xFF, FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_ACCEPTFOOTER, 
					FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_ACCEPTFOOTER], 0xFF, FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}

	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_SYOMEI, 
					FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_SYOMEI], 0xFF, FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}

	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_KAMEI, 
					FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_KAMEI], 0xFF, FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}

// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EDYAT, 
//					FLT_HEADER_SIZE+FLT_EDYATCMD_SIZE, 
//					&blank);
//	if (!sum && !blank) {
//	/* �T��NG�Ȃ̂Ƀf�[�^���� */
//		memset(&wbuf[FLT_PRINTOFS_EDYAT], 0xFF, FLT_HEADER_SIZE+FLT_EDYATCMD_SIZE);			/* �f�[�^�C���[�X */
//		write = TRUE;
//	}
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_USERDEF, 
					FLT_HEADER_SIZE+FLT_USERDEF_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_USERDEF], 0xFF, FLT_HEADER_SIZE+FLT_USERDEF_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}

	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_TCARDFTR, 
					FLT_HEADER_SIZE+FLT_TCARDFTR_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_TCARDFTR], 0xFF, FLT_HEADER_SIZE+FLT_TCARDFTR_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_AZUFTR, 
					FLT_HEADER_SIZE+FLT_AZUFTR_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_AZUFTR], 0xFF, FLT_HEADER_SIZE+FLT_AZUFTR_SIZE);		/* �f�[�^�C���[�X */
		write = TRUE;
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EMGFOOTER, 
					FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* �T��NG�Ȃ̂Ƀf�[�^���� */
		memset(&wbuf[FLT_PRINTOFS_EMGFOOTER], 0xFF, FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE);	/* �f�[�^�C���[�X */
		write = TRUE;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	if (write) {
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
	/* �f�[�^�������� */
		if (_FLT_RtnKind(flt_WritePrintDataSector(wbuf)) != FLT_NORMAL) {
			flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
		}
		req_tbl->req = FLT_REQ_IDLE;
	}
	
	return;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CheckData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�`�F�b�N
 *| param	: top_address - �擪�A�h���X
 *|			  size - �T�C�Y
 *|			  blank - TRUE=�u�����N						<OUT>
 *| return	: TRUE - �T��OK
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckData(ulong top_address, ulong size, BOOL *blank)
{
	*blank = FALSE;
	
	if (!flt_CheckSum(top_address, size)) {
	/* �T�����Ȃ� */
		if (Flash2ChkBlank(top_address, size)) {
			*blank = TRUE;
		}
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CheckSum()
 *[]----------------------------------------------------------------------[]
 *| summary	: �T���`�F�b�N
 *| param	: top_address - �擪�A�h���X
 *|			  size - �T�C�Y
 *| return	: TRUE - �T��OK
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckSum(ulong top_address, ulong size)
{
	uchar	c[FLT_HEADER_SIZE];
	
	/* SUM�ǂݍ��� */
	Flash2Read(c, top_address, sizeof(c));
	
	if( (c[0]^c[1])!= 0xFF ){	// �T�����Ȃ�
		return FALSE;
	}
	else if (c[0] != Flash2Sum(top_address+FLT_HEADER_SIZE, size-FLT_HEADER_SIZE)) {	// SUM�ُ�
		return FALSE;
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParam1()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^��������(�����Ƀo�C�i���C���[�W����)
 *| param	: param_tbl - �p�����[�^���e�[�u��		<IN>
 *|			  param_tbl_wk - ���[�N�p�p�����[�^���e�[�u��		<WORK>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *|			  cmp - �������ݑO��ł̔�r���@(FLT_CMP_ALL/FLT_CMP_CPRM_ONLY)
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParam1(const t_AppServ_ParamInfoTbl *param_tbl, 
						t_AppServ_ParamInfoTbl *param_tbl_wk, 
						uchar *wbuf, t_FltReq *req_tbl, uchar cmp_method)
{
	ulong len;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* �v����t�̃`�F�b�N�|�C���g */

	if( prm_invalid_check() ){
		DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
	}
	
/* RAM��FLASH�̔�r */
	if (flt_CmpParamData(param_tbl, param_tbl_wk, cmp_method) != 0) {
/* �������݃C���[�W�쐬 */
		memset(wbuf, 0xFF, (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX));
		len = FLT_HEADER_SIZE;
		/* �p�����[�^���e�[�u�� */
		len += flt_WriteParamTbl(&wbuf[len], param_tbl);
		if (len + flt_CParamDataSize(param_tbl) > (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
			return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
		}
		/* �p�����[�^�f�[�^�{�� */
		flt_ParamCopy(&wbuf[len], param_tbl);
		/* �T�� */
		flt_SetSum(wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
/* �������ݎ��s */
		ret = flt_WriteParamSector(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	}
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C(�ŐV�̃f�[�^�{�̂Ɣ�r) */
		if (flt_CmpParamData(param_tbl, param_tbl_wk, cmp_method) != 0) {
			flt_EraseSectors(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
			ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CmpParamData()
 *[]----------------------------------------------------------------------[]
 *| summary	: RAM��FLASH �p�����[�^�f�[�^���e�̔�r
 *| param	: param_tbl - �p�����[�^���e�[�u��		<IN>
 *|			  param_tbl_wk - ���[�N�p�p�����[�^���e�[�u��		<WORK>
 *|			  cmp - �������ݑO��ł̔�r���@(FLT_CMP_ALL/FLT_CMP_CPRM_ONLY)
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CmpParamData(const t_AppServ_ParamInfoTbl *param_tbl, 
						t_AppServ_ParamInfoTbl *param_tbl_wk, 
						uchar cmp_method)
{
	ushort section;
/* �T���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
		return 1;
	}
/* �t���b�V����̃o�b�N�A�b�v���e�[�u����ǂݍ��� */
	flt_ReadParamTbl(param_tbl_wk);
/* ���ʃp�����[�^ */
	/* �Z�N�V������ */
	if (param_tbl->csection_num != param_tbl_wk->csection_num) {
		return 1;
	}
	for (section = 0; section < param_tbl->csection_num; section++) {
	/* �e�Z�N�V�����̍��ڐ� */
		if (param_tbl->csection[section].item_num != param_tbl_wk->csection[section].item_num) {
			return 1;
		}
	/* �e�Z�N�V�����̃f�[�^���e */
		if (Flash2Cmp((ulong)param_tbl_wk->csection[section].address + FLT_PARAM_SECTOR,
					param_tbl->csection[section].address,
					param_tbl->csection[section].item_num * FLT_PARAM_DATA_SIZE) != 0) {
			return 1;
		}
	}
	
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParam2()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^��������(�����Ƀo�C�i���C���[�W�t��)
 *| param	: params - API���[�U����̈���				<IN>
 *|			  param_tbl - �p�����[�^���e�[�u��		<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParam2(const t_FltParam_Param2 *params, 
					const t_AppServ_ParamInfoTbl *param_tbl, 
					uchar *wbuf,
					t_FltReq *req_tbl)
{
	ulong len;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* �������݃C���[�W�쐬 */
	memset(wbuf, 0xFF, (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX));
	len = FLT_HEADER_SIZE;
	/* �p�����[�^���e�[�u�� */
	len += flt_WriteParamTbl(&wbuf[len], param_tbl);
	if (len + params->image_size > FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
	/* �p�����[�^�f�[�^�{�� */
	memcpy(&wbuf[len], params->param_image, params->image_size);
	/* �T�� */
	flt_SetSum(wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	
/* �������ݐ�C���[�W�Ɣ�r */
	if (Flash2Cmp(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WriteParamSector(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) != 0) {
				flt_EraseSectors(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
		_flt_DestroyParamRamSum();		/* RAM��̃p�����[�^�T�����N���A */
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���O��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  logmgr - LOG�Ǘ��f�[�^					<IN/OUT>
 *|			  logmgr_bk - ���d�����p�̃o�b�N�A�b�v�̈�	<OUT>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *|			  clr_kind - RAM�f�[�^�폜�R�[�h
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLog(t_FltParam_Log *params, t_FltLogMgr *logmgr, t_FltReq *req_tbl, short clr_kind)
{
	ulong	ret;
	ushort	sector;
	ushort	seqno;
	int	i;
	struct log_record	*lp = LOG_DAT+clr_kind;

	params->stat_Fcount = lp->Fcount;
	params->stat_sectnum = logmgr->sectnum;
	params->f_recover = 1;// �����J�n
/* �����ݏ��쐬 */
	flt_GetWriteLogSector(logmgr, &sector, &seqno);
	params->get_sector = sector;
	params->get_seqno = seqno;
/* �����ݎ��s */
	if(logmgr->sectinfo[sector].address >= FLT_WAVE_SWDATA_SECTOR){
		// �����f�[�^�^�p�ʏ��̗̈�ȍ~�ɃA�N�Z�X�����ꍇ�������Ȃ�
		// �o�b�N�A�b�v�o�b�e���[�̃W�����p��OPEN�̏�ԂŋN���������A���O�̊Ǘ��̈悪���ilogmgr->sectinfo[sector].address�j
		// �ɉ����f�[�^�̗̈���w�肳�ꂽ���A�����f�[�^��j�󂵂Ȃ��悤�ɂ��邽�߂����Đ���I����Ԃ�
		return _FLT_MkRtnVal2(FLT_NORMAL);
	}
	ret = flt_WriteLogData(clr_kind, params, logmgr, sector, seqno);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		goto _err;
	}
/* �x���t�@�C */
	if (flt_CmpLogData(clr_kind, params, logmgr, sector, seqno) != 0) {
	// ���̏ꍇ���������݃G���[�o�^����
		ret = FROM2_ADDRESS+logmgr->sectinfo[sector].address;
		FlashRegistErr(ERR_MAIN_FLASH_WRITE_ERR, ret);
		ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
		goto _err;
	}
	params->f_recover = 2;// �������ݏI��
/* LOG����f�[�^�X�V */
	if( lp->Fcount < logmgr->sectmax ){
		lp->Fcount++;
		params->f_recover = 3;// ���C�g�|�C���^�X�V
	}
	logmgr->sectinfo[sector].seqno		= seqno;
	logmgr->sectinfo[sector].record_num	= params->record_num;
	if (logmgr->sectnum + 1 > logmgr->sectmax) {
		params->f_recover = 40;// �Z�N�^����MAX�𒴂���P�[�X
		/* ���ɑS�Z�N�^�Ƀf�[�^���������P�[�X */
		logmgr->secttop = _offset(logmgr->secttop, 1, logmgr->sectmax);
		for(i=0; i<eLOG_TARGET_MAX; i++){
			if( lp->unread[i]>LOG_SECORNUM(clr_kind) ){
				lp->overWriteUnreadCount[i] = LOG_SECORNUM(clr_kind);
				lp->unread[i] -= LOG_SECORNUM(clr_kind);
			}
			else{
				lp->overWriteUnreadCount[i] = lp->unread[i];
				lp->unread[i] = 0;
			}
		}
	}
	else {
		params->f_recover = 41;// �Z�N�^����MAX�𒴂��Ȃ��P�[�X
		logmgr->sectnum++;
	}
/* RAM���LOG�f�[�^�N���A */
	// �������񂾑���SRAM���LOG(4Kbyte2�ʂ̂ǂ���̏��������������������f�[�^)�̃J�E���g����������
	LOG_DAT[clr_kind].count[params->sramlog_kind] = 0;
	params->f_recover = 0;// �����I��
	return _FLT_MkRtnVal2(FLT_NORMAL);
	
_err:
	/* ������/�x���t�@�C�Ɏ��s�����Z�N�^�͖����Ƃ��� */
	Flash2EraseSector(logmgr->sectinfo[sector].address);
	logmgr->sectinfo[sector].record_num = 0;
	if (logmgr->sectnum >= logmgr->sectmax) {
		logmgr->secttop = _offset(logmgr->secttop, 1, logmgr->sectmax);
		logmgr->sectnum--;
	}
	// �������񂾑���SRAM���LOG(4Kbyte2�ʂ̂ǂ���̏��������������������f�[�^)�̃J�E���g����������
	LOG_DAT[clr_kind].count[params->sramlog_kind] = 0;
	params->f_recover = 0;// �����I��
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_GetWriteLogSector()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG�f�[�^�����ݐ�Z�N�^���擾
 *| param	: logmgr - LOG�Ǘ��f�[�^									<IN>
 *|			  sector - �������ݐ�Z�N�^�ԍ�(0�`logmgr->sectmax-1)		<OUT>
 *|			  seqno - �����ݐ�Z�N�^�̒ǂ���							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
void	flt_GetWriteLogSector(const t_FltLogMgr *logmgr, ushort *sector, ushort *seqno)
{
	/* �������ݐ�Z�N�^�擾 */
	*sector	= _offset(logmgr->secttop, logmgr->sectnum, logmgr->sectmax);
	/* �������ݐ�Z�N�^�̒ǂ��Ԏ擾 */
	if (logmgr->sectnum <= 0) {
		*seqno = 0;
	}
	else {
		/* �ЂƂO�̃Z�N�^�̒ǂ���+1 */
		*seqno = logmgr->sectinfo[_offset(*sector, logmgr->sectmax-1, logmgr->sectmax)].seqno;
		if( ++(*seqno)>=0xffff ){
			*seqno = 0;
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLogData()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���O��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  logmgr - LOG�Ǘ��f�[�^					<IN/OUT>
 *|			  sector - �������ݐ�Z�N�^�ԍ�(0�`logmgr->sectmax-1)
 *|			  seqno - �����ݐ�Z�N�^�̒ǂ���
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLogData(ushort Lno, const t_FltParam_Log *params, const t_FltLogMgr *logmgr, ushort sector, ushort seqno)
{
	ulong	ret, dst;
	typedef struct{
		char	Sum[2];
		ushort	record_num;
		ushort	seqno;
	} t_log_inf;
	union{
		uchar		c_buf[FLT_LOGOFS_RECORD];
		t_log_inf	log_inf;
	}u;
	ulong	dsize;

	u.log_inf.seqno = seqno;
	u.log_inf.record_num = params->record_num;
/* �����ݐ�Z�N�^�C���[�X */
	dst = logmgr->sectinfo[sector].address;
	ret = Flash2EraseSector2(dst);
	if (ret != FROM2_NORMAL) {
		return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
	}
	if( logmgr->record_size>RECODE_SIZE ){
		ret = Flash2EraseSector2(dst+FLT_LOGSECT_SIZE);
		if (ret != FROM2_NORMAL) {
			return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
		}
	}
	dsize = LogDatMax[Lno][1];

	memcpy(FLASH_WRITE_BUFFER, &u.c_buf, FLT_LOGOFS_RECORD);
	// ���R�[�h
	memcpy(FLASH_WRITE_BUFFER+FLT_LOGOFS_RECORD, params->records, dsize);
	// �T��
	FLASH_WRITE_BUFFER[0] = flt_CalcSum(FLASH_WRITE_BUFFER+FLT_HEADER_SIZE, 3+dsize);
	FLASH_WRITE_BUFFER[1] = (uchar)~FLASH_WRITE_BUFFER[0];

/* ������ */
	ret = Flash2Write(dst, FLASH_WRITE_BUFFER, FLT_LOGOFS_RECORD+dsize);
	if (ret != FROM2_NORMAL) {
		return  _FLT_MkRtnVal(FLT_WRITE_ERR, ret);
	}

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CmpLogData()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG�f�[�^���e��r(FLASH��RAM)
 *| param	: params - API���[�U����̈���				<IN>
 *|			  logmgr - LOG�Ǘ��f�[�^					<IN/OUT>
 *|			  sector - �������݂��s�����Z�N�^�̔ԍ�(0�`logmgr->sectmax-1)
 *|			  seqno - �����݂��s�����Z�N�^�̒ǂ���
 *| return	: 0=���ᖳ��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CmpLogData(ushort Lno, const t_FltParam_Log *params, const t_FltLogMgr *logmgr, ushort sector, ushort seqno)
{
	ulong	addr;
	ushort	record_num, i;
	ulong	dsize;

	dsize = LogDatMax[Lno][1]+FLT_LOGOFS_RECORD;

	record_num = params->record_num;

	/* �T���̃`�F�b�N */
	addr = logmgr->sectinfo[sector].address;
	if( !flt_CheckSum(addr, dsize) ){
		return 1;
	}
	addr += FLT_HEADER_SIZE;
	/* ���R�[�h�� */
	if (Flash2Cmp(addr, (uchar*)&record_num, 2) != 0) {
		return 1;
	}
	addr+=2;
	/* �ǂ��� */
	if( Flash2Cmp(addr, (uchar*)&seqno, 2)!=0 ){
		return 1;
	}
	addr += 2;
	/* ���R�[�h */
	for (i = 0; i < record_num; i++) {
		if (Flash2Cmp(addr, (uchar*)&params->records[logmgr->record_size*i], (ulong)logmgr->record_size) != 0) {
			return 1;
		}
		addr += logmgr->record_size;
	}
	
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLogo()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g���S��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLogo(const t_FltParam_Logo *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong size;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
/* �������݃f�[�^�����`�F�b�N */
	size = get_bmplen((uchar*)params->image);
	if (size <= 0 || FLT_PRINTLOGO_SIZE < size) {
		return FLT_INVALID_SIZE;
	}
/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_LOGO+FLT_HEADER_SIZE], params->image, FLT_PRINTLOGO_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_LOGO], FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteHeader()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�w�b�_��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteHeader(const t_FltParam_Header *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_HEADER+FLT_HEADER_SIZE], params->image, FLT_PRINTHEADER_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_HEADER], FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�t�b�^��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteFooter(const t_FltParam_Footer *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_FOOTER+FLT_HEADER_SIZE], params->image, FLT_PRINTFOOTER_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_FOOTER], FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteAcceptFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��t���t�b�^��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteAcceptFooter(const t_FltParam_AcceptFooter *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_ACCEPTFOOTER+FLT_HEADER_SIZE], params->image, FLT_ACCEPTFOOTER_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_ACCEPTFOOTER], FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLockParam1()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��p�����[�^��������(�����Ƀo�C�i���C���[�W����)
 *| param	: wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLockParam1(uchar *wbuf, t_FltReq *req_tbl)
{
	union {
		ushort	s;
		uchar	c[2];
	}u;
	ulong size;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* �v����t�̃`�F�b�N�|�C���g */
	
/* �f�[�^���`�F�b�N */
	size = FLT_LKPRMOFS_PARAM + sizeof(LockInfo);
	if (size > FLT_LOCKPARAM_SECT_SIZE) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
/* RAM��FLASH�̔�r */
	if (flt_CmpLockParamData() != 0) {
/* �������݃C���[�W�쐬 */
		memset(wbuf, 0xFF, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
		/* �Ԏ��� */
		u.s = LOCK_MAX;
		wbuf[FLT_LKPRMOFS_LOCKMAX]		= u.c[0];
		wbuf[FLT_LKPRMOFS_LOCKMAX+1]	= u.c[1];
		/* �Ԏ����̃f�[�^�T�C�Y */
		u.s = sizeof(t_LockInfo);
		wbuf[FLT_LKPRMOFS_DATASIZE]		= u.c[0];
		wbuf[FLT_LKPRMOFS_DATASIZE+1]	= u.c[1];
		/* �Ԏ��p�����[�^�{�� */
		memcpy(&wbuf[FLT_LKPRMOFS_PARAM], LockInfo, sizeof(LockInfo));
		/* �T�� */
		flt_SetSum(wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);

/* �������ݎ��s */
		ret = flt_WriteParamSector(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
	}
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C(�ŐV�̃f�[�^�{�̂Ɣ�r) */
		if (flt_CmpLockParamData() != 0) {
			flt_EraseSectors(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE, FLT_LOCKPARAM_SECTORMAX);
			ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CmpLockParamData()
 *[]----------------------------------------------------------------------[]
 *| summary	: RAM��FLASH �Ԏ��p�����[�^�f�[�^���e�̔�r
 *| param	: none
 *| return	: 0�����ᖳ��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CmpLockParamData(void)
{
	ushort	lockmax, datasize;
	
/* �T���̃`�F�b�N */
	if (!flt_CheckSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX)) {
		return 1;
	}
/* �t���b�V����̃f�[�^�Ǘ�����ǂݍ��� */
	Flash2Read((uchar*)&lockmax, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_LOCKMAX, sizeof(lockmax));
	Flash2Read((uchar*)&datasize, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_DATASIZE, sizeof(datasize));
/* �Ԏ��� */
	if (lockmax != LOCK_MAX) {
		return 1;
	}
/* �Ԏ����̃f�[�^�T�C�Y */
	if (datasize != sizeof(t_LockInfo)) {
		return 1;
	}
/* �Ԏ��p�����[�^�̓��e */
	if (Flash2Cmp(FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_PARAM, (uchar*)LockInfo, sizeof(LockInfo)) != 0) {
		return 1;
	}
	
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLockParam2()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��p�����[�^��������(�����Ƀo�C�i���C���[�W�t��)
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLockParam2(const t_FltParam_LockParam2 *params, uchar *wbuf, t_FltReq *req_tbl)
{
	union {
		ushort	s;
		uchar	c[2];
	}u;
	ulong size;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* �f�[�^���`�F�b�N */
	size = FLT_LKPRMOFS_PARAM + sizeof(LockInfo);
	if (size > FLT_LOCKPARAM_SECT_SIZE) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
/* �������݃C���[�W�쐬 */
	memset(wbuf, 0xFF, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
	/* �Ԏ��� */
	u.s = LOCK_MAX;
	wbuf[FLT_LKPRMOFS_LOCKMAX]		= u.c[0];
	wbuf[FLT_LKPRMOFS_LOCKMAX+1]	= u.c[1];
	/* �Ԏ����̃f�[�^�T�C�Y */
	u.s = sizeof(t_LockInfo);
	wbuf[FLT_LKPRMOFS_DATASIZE]		= u.c[0];
	wbuf[FLT_LKPRMOFS_DATASIZE+1]	= u.c[1];
	/* �p�����[�^�f�[�^�{�� */
	memcpy(&wbuf[FLT_LKPRMOFS_PARAM], params->param_image, sizeof(LockInfo));
	/* �T�� */
	flt_SetSum(wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
	
/* �������ݐ�C���[�W�Ɣ�r */
	if (Flash2Cmp(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WriteParamSector(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX) != 0) {
				flt_EraseSectors(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE, FLT_LOCKPARAM_SECTORMAX);
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
		_flt_DestroyLockParamRamSum();		/* RAM��̃p�����[�^�T�����N���A */
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Backup()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v���s
 *| param	: salelog_params - API���[�U����̈���				<IN>
 *|			  totallog_params - API���[�U����̈���				<IN>
 *|			  param_tbl - �p�����[�^���e�[�u��				<IN>
 *|			  area_tbl - �o�b�N�A�b�v�f�[�^���e�[�u��			<IN>
 *|			  area_tbl_wk - ���[�N�p�o�b�N�A�b�v���e�[�u��	<WORK>
 *|			  wbuf - ���[�N�o�b�t�@								<WORK>
 *|			  loginfo - LOG�Ǘ��f�[�^							<IN/OUT>
 *|			  loginfo_wk - LOG�Ǘ��f�[�^�̕��d�p�o�b�N�A�b�v	<OUT>
 *|			  req_tbl - �v����t���							<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_Backup(
				const t_FltParam_Bkup *bkup_params,
				const t_AppServ_ParamInfoTbl *param_tbl, 
				t_AppServ_ParamInfoTbl *param_tbl_wk,
				const t_AppServ_AreaInfoTbl *area_tbl,
				t_AppServ_AreaInfoTbl *area_tbl_wk,
				uchar *wbuf)
{
	t_FltReq dummy;
	ulong ret;

/* �e�̈�̃o�b�N�A�b�v */
	/* �ݒ�p�����[�^ */
	ret = flt_WriteParam1(param_tbl, param_tbl_wk, wbuf, &dummy, FLT_CMP_ALL);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* �Ԏ��p�����[�^ */
	ret = flt_WriteLockParam1(wbuf, &dummy);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* �o�b�N�A�b�v�f�[�^ */
	ret = flt_WriteBackupData(area_tbl, area_tbl_wk, param_tbl, bkup_params->ver_flg);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteBackupData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v�f�[�^�̈揑������
 *| param	: area_tbl - �o�b�N�A�b�v�f�[�^���e�[�u��				<IN>
 *|			  area_tbl_wk - ���[�N�p�o�b�N�A�b�v�f�[�^���e�[�u��	<WORK>
 *| return	: 0=���ᖳ��
 *[]----------------------------------------------------------------------[]*/
static ulong	flt_WriteBackupData(const t_AppServ_AreaInfoTbl *area_tbl,
								t_AppServ_AreaInfoTbl *area_tbl_wk,
								const t_AppServ_ParamInfoTbl *param_tbl,
								ulong ver_flg)
{
	ulong ret;
	
	/* dst��src�̔�r */
	if (flt_CmpBackupData(area_tbl, area_tbl_wk, param_tbl, ver_flg) != 0) {
	/* �������� */
		ret = flt_WriteBackupDataSector(area_tbl, param_tbl, ver_flg);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			flt_EraseSectors(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE, FLT_BACKUP_SECTORMAX);
			return ret;
		}
	/* �x���t�@�C(�ŐV�̃f�[�^�{�̂Ɣ�r) */
		if (flt_CmpBackupData(area_tbl, area_tbl_wk, param_tbl, ver_flg) != 0) {
			flt_EraseSectors(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE, FLT_BACKUP_SECTORMAX);
			return _FLT_MkRtnVal2(FLT_VERIFY_ERR);
		}
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CmpBackupData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v�f�[�^�̔�r(RAM��FLASH)
 *| param	: area_tbl - �o�b�N�A�b�v�f�[�^���e�[�u��				<IN>
 *|			  area_tbl_wk - ���[�N�p�o�b�N�A�b�v�f�[�^���e�[�u��	<WORK>
 *| return	: 0=���ᖳ��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CmpBackupData(const t_AppServ_AreaInfoTbl *area_tbl, 
						t_AppServ_AreaInfoTbl *area_tbl_wk,
						const t_AppServ_ParamInfoTbl *param_tbl,
						ulong ver_flg)
{
	ushort area;
	ulong	src_addr,src_addr2;
	ushort	section_num, section;
	t_AppServ_SectInfo sectinfo;
	
	/* �T���̃`�F�b�N */
	// �e�[�u�����̃T���`�F�b�N
	if (!flt_CheckSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE)) {
		return 1;
	}
	/* �t���b�V����̃o�b�N�A�b�v���e�[�u����ǂݍ��� */
	flt_ReadAreaTbl(area_tbl_wk);
	/* �G���A���̔�r */
	if (area_tbl->area_num != area_tbl_wk->area_num) {
		return 1;
	}
	for (area = 0; area < area_tbl->area_num; area++) {
	/* �e�G���A�̃T�C�Y�̔�r */
		if (area == OPE_DTNUM_LOCKINFO) {
			if (area_tbl_wk->area[area].size == 0) {
				continue;
			}
		}
		if (area_tbl->area[area].size != area_tbl_wk->area[area].size) {
			return 1;
		}
	/* �e�G���A�̃f�[�^�̔�r */
		if (area == FLT_DTNUM_MAPVER) {
			if ((ulong)area_tbl_wk->area[area].address != FLASH2_MAP_VERSION) {
				return 1;
			}
		}
		else
		if (area == FLT_DTNUM_VERUPFLG) {
			if ((ulong)area_tbl_wk->area[area].address != ver_flg) {
				return 1;
			}
		}
		else {
			if (Flash2Cmp(area_tbl_wk->from_area[area].from_address,
						area_tbl->area[area].address,
						area_tbl->area[area].size) != 0) {
				return 1;
			}
		}
	}
	/* �t���b�V����̌ʃp�����[�^���e�[�u���Ǎ��� */
	src_addr = FLT_BACKUP_SECTOR26 + FLT_HEADER_SIZE;
	src_addr2 = FLT_BACKUP_SECTOR27 + FLT_HEADER_SIZE;
	/* �ʃp�����[�^�Z�N�V�������̔�r */
	Flash2Read((uchar*)&section_num, src_addr, sizeof(section_num));
	if (section_num == 0xFFFF || section_num != param_tbl->psection_num) {
		return 1;
	}
	src_addr += sizeof(section_num);
	for (section = 0; section < section_num; section++) {
		Flash2Read((uchar*)&sectinfo, src_addr, sizeof(sectinfo));
	/* �Z�N�V������f�[�^���̔�r */
		if (sectinfo.item_num != param_tbl->psection[section].item_num) {
			return 1;
		}
	/* �ʃp�����[�^�Z�N�V�������̔�r */
		if (Flash2Cmp(src_addr2,
					param_tbl->psection[section].address,
					sectinfo.item_num * FLT_PARAM_DATA_SIZE) != 0) {
			return 1;
		}
		src_addr += sizeof(sectinfo);
		src_addr2 += sectinfo.item_num * FLT_PARAM_DATA_SIZE;
	}
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteBackupDataSector()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v�f�[�^�̈�̏�������
 *| param	: area_tbl - �o�b�N�A�b�v�f�[�^���e�[�u��		<IN>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteBackupDataSector(const t_AppServ_AreaInfoTbl *area_tbl, const t_AppServ_ParamInfoTbl *param_tbl, ulong ver_flg)
{
	ulong	ret;
	
	/* �o�b�N�A�b�v�f�[�^�̈�C���[�X */
	ret = flt_EraseSectors(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE, FLT_BACKUP_SECTORMAX);
	if( ret != FLT_NORMAL ){
		return ret;
	}
	/* �e�[�u���������� */
	ret = flt_WriteAreaTbl(area_tbl, ver_flg);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* �o�b�N�A�b�v�f�[�^�������� */
	ret = flt_WriteAreaData(area_tbl);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* �ʃp�����[�^�e�[�u�������� */
	ret = flt_WritePParamTbl(param_tbl, FLT_BACKUP_SECTOR26);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* �ʃp�����[�^������ */
	ret = flt_WritePParamData(param_tbl, FLT_BACKUP_SECTOR27);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WritePParamTbl()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ʃp�����[�^���e�[�u���̃o�b�N�A�b�v�̈�ւ̏�������
 *| param	: param_tbl - �p�����[�^���e�[�u��		<IN>
 *|			  addr -  �������݊J�n�A�h���X
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WritePParamTbl(const t_AppServ_ParamInfoTbl *param_tbl, ulong addr)
{
	ulong ofs, section, ret;
	t_AppServ_SectInfo	w;
	
	memset(&FLASH_WRITE_BUFFER[0] ,0xFF ,sizeof(FLASH_WRITE_BUFFER));
	ofs = FLT_HEADER_SIZE;
	/* �Z�N�V������ */
	memcpy(&FLASH_WRITE_BUFFER[ofs],(uchar*)&param_tbl->psection_num,sizeof(param_tbl->psection_num));
	ofs += sizeof(param_tbl->psection_num);
	/* �ʃp�����[�^�e�[�u����� */
	for (section = 0; section < param_tbl->psection_num; section++) {
		w = param_tbl->psection[section];
		memcpy(&FLASH_WRITE_BUFFER[ofs],(uchar*)&w,sizeof(t_AppServ_SectInfo));
		ofs += sizeof(w);
	}
	flt_SetSum(&FLASH_WRITE_BUFFER[0],FLT_BACKUP_SECT_SIZE);
	ret = Flash2Write(addr, &FLASH_WRITE_BUFFER[0], FLT_BACKUP_SECT_SIZE);
	if (ret != FROM2_NORMAL) {
		return _FLT_MkRtnVal(FLT_WRITE_ERR, ret);
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WritePParamData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ʃp�����[�^�̃o�b�N�A�b�v�̈�ւ̏�������
 *| param	: param_tbl - �p�����[�^���e�[�u��		<IN>
 *|			  addr -  �������݊J�n�A�h���X
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WritePParamData(const t_AppServ_ParamInfoTbl *param_tbl, ulong addr)
{
	ushort	section;
	ulong	copysize, ofs, ret;

	/* �ʃp�����[�^ */
	memset(&FLASH_WRITE_BUFFER[0] ,0xFF ,sizeof(FLASH_WRITE_BUFFER));
	ofs = FLT_HEADER_SIZE;
	for (section = 0; section < param_tbl->psection_num; section++) {
		copysize = param_tbl->psection[section].item_num * FLT_PARAM_DATA_SIZE;
		if (copysize > 0) {
			memcpy(&FLASH_WRITE_BUFFER[ofs],(uchar*)param_tbl->psection[section].address,copysize);
			ofs += copysize;
		}
	}

	flt_SetSum(&FLASH_WRITE_BUFFER[0],FLT_BACKUP_SECT_SIZE);
	ret = Flash2Write(addr, &FLASH_WRITE_BUFFER[0], FLT_BACKUP_SECT_SIZE);
	if (ret != FROM2_NORMAL) {
		return _FLT_MkRtnVal(FLT_WRITE_ERR, ret);
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CParamDataSize()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ʃp�����[�^�̃f�[�^�T�C�Y�擾
 *| param	: param_tbl - �p�����[�^���e�[�u��		<IN>
 *| return	: ���ʃp�����[�^�f�[�^�T�C�Y
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CParamDataSize(const t_AppServ_ParamInfoTbl *param_tbl)
{
	ulong size, section;
	
	size = 0;
	for (section = 0; section < param_tbl->csection_num; section++) {
		size += (param_tbl->csection[section].item_num * FLT_PARAM_DATA_SIZE);
	}
	
	return size;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteAreaData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v�f�[�^���t���b�V���ɏ�������
 *| param	: area_tbl - �o�b�N�A�b�v�f�[�^���e�[�u��			<IN>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteAreaData(const t_AppServ_AreaInfoTbl *area_tbl)
{
	ulong ret, area, dst;
	
	/* �f�[�^�������� */
	for (area = 0; area < area_tbl->area_num; area++) {
		if (area == OPE_DTNUM_LOCKINFO) {
			continue;
		}
		dst = area_tbl->from_area[area].from_address;
		if(area_tbl->from_area[area].from_address == 0L){
			continue;// �A�h���X�������ꍇ�͊i�[�ΏۊO
		}
		ret = Flash2Write(dst, area_tbl->area[area].address, area_tbl->area[area].size);
		if (ret != FROM2_NORMAL) {
			return _FLT_MkRtnVal(FLT_WRITE_ERR, ret);
		}
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_RestoreParam()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ݒ�p�����[�^���X�g�A
 *| param	: param_tbl - �ŐV�̃p�����[�^���e�[�u��		<IN>
 *|			  param_tbl_wk - ���[�N�p�p�����[�^���e�[�u��	<WORK>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_RestoreParam(const t_AppServ_ParamInfoTbl *param_tbl, t_AppServ_ParamInfoTbl *param_tbl_wk)
{
	ushort	section, section_num, item_num;
	
/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2);
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	
/* �t���b�V������e�[�u���ǂݍ��� */
	flt_ReadParamTbl(param_tbl_wk);
	
/* ���ʃp�����[�^ */
	/* �Z�N�V�������̌���(���Ȃ��ق��ɍ��킹��) */
	section_num = param_tbl_wk->csection_num;
	if (param_tbl->csection_num < param_tbl_wk->csection_num) {
		section_num = param_tbl->csection_num;
	}
	for (section = 0; section < section_num; section++) {
		/* ���ڐ��̌���(���Ȃ��ق��ɍ��킹��) */
		item_num = param_tbl_wk->csection[section].item_num;
		if (param_tbl->csection[section].item_num < param_tbl_wk->csection[section].item_num) {
			item_num = param_tbl->csection[section].item_num;
		}
		/* RAM�ւ̃f�[�^�]�� */
		Flash2Read(param_tbl->csection[section].address,
				(ulong)param_tbl_wk->csection[section].address + FLT_PARAM_SECTOR,
				item_num * FLT_PARAM_DATA_SIZE);
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_RestoreLockParam()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Ԏ��p�����[�^���X�g�A
 *| param	: none
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_RestoreLockParam(void)
{
	ushort	lockmax, datasize, copysize, i;
	
/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	
/* �t���b�V����̃f�[�^�Ǘ�����ǂݍ��� */
	Flash2Read((uchar*)&lockmax, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_LOCKMAX, sizeof(lockmax));
	Flash2Read((uchar*)&datasize, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_DATASIZE, sizeof(datasize));
/* �Ԏ����E�Ԏ����f�[�^�T�C�Y�̌���(���Ȃ��ق��ɍ��킹��) */
	if (lockmax > LOCK_MAX) {
		lockmax = LOCK_MAX;
	}
	copysize = datasize;
	if (copysize > sizeof(t_LockInfo)) {
		copysize = sizeof(t_LockInfo);
	}
/* ���X�g�A���s */
	memset(LockInfo, 0, sizeof(LockInfo));
	for (i = 0; i < lockmax; i++) {
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		Flash2Read((uchar*)&LockInfo[i], 
				(ulong)(FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_PARAM+(datasize*i)), (ulong)copysize);
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_GetLogRecordNum()
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V�����LOG���R�[�h�����擾
 *| param	: logmgr - LOG�Ǘ��f�[�^					<IN>
 *| return	: LOG���R�[�h����
 *[]----------------------------------------------------------------------[]*/
long flt_GetLogRecordNum(const t_FltLogMgr *logmgr)
{
	ushort	i, ofs;
	long	record_num;

	record_num = 0;
	for (i = 0; i < logmgr->sectnum; i++) {
		ofs = _offset(logmgr->secttop, i, logmgr->sectmax);
		record_num += logmgr->sectinfo[ofs].record_num;
	}

	return record_num;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadLogRecord()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG���R�[�h�擾
 *| param	: logmgr - LOG����f�[�^				<IN>
 *|			  handle - LOG�����n���h��				<IN>
 *|			  buf - LOG�擾�o�b�t�@					<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadLogRecord(const t_FltLogMgr *logmgr, const t_FltLogHandle *handle, char *buf)
{
	ulong src_addr;
	
	if (logmgr->sectmax <= handle->sectno) {
		return _FLT_MkRtnVal2(FLT_NODATA);	/* �w��ʒu�Ƀ��R�[�h���� */
	}
	if (handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		return _FLT_MkRtnVal2(FLT_NODATA);	/* �w��ʒu�Ƀ��R�[�h���� */
	}
	
	/* �f�[�^�ǂݏo�� */
	src_addr = logmgr->sectinfo[handle->sectno].address 
				+ FLT_LOGOFS_RECORD
				+ (handle->recordno * logmgr->record_size);
	Flash2Read((uchar*)buf, src_addr, (ulong)logmgr->record_size);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}


/*[]----------------------------------------------------------------------[]
 *|	name	: flt_NextLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG�����n���h���X�V
 *| param	: logmgr - LOG����f�[�^					<IN>
 *|			  handle - LOG�����n���h��					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_NextLog(const t_FltLogMgr *logmgr, t_FltLogHandle *handle)
{
	if (handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		return _FLT_MkRtnVal2(FLT_NODATA);	/* ���݈ʒu�Ƀ��R�[�h���� */
	}
	/* �����R�[�h�ֈړ� */
	if (++handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		/* ���̃Z�N�^�̃��R�[�h���I���ł���Ύ��Z�N�^�ֈړ� */
		handle->sectno = _offset(handle->sectno, 1, logmgr->sectmax);
		if (handle->sectno == _offset(logmgr->secttop, logmgr->sectnum, logmgr->sectmax)) {
			return _FLT_MkRtnVal2(FLT_NODATA);	/* �����R�[�h�Ȃ� */
		}
		handle->recordno = 0;
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadLogo()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g���S�擾
 *| param	: buf 							<OUT>
 *| 		  RaedType : 0=�S���ǂݏo���A1=�����Ǐo��
 *| 		  TopOffset,ReadSize : �����Ǐo���̏ꍇ�̂ݗL���B
 *| 		  					   �Ǐo���J�n�I�t�Z�b�g�ƃT�C�Y�B
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadLogo(char *buf, char RaedType, ushort TopOffset, ushort ReadSize)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_LOGO, FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	if( 1 == RaedType ){	// �����Ǐo�� (Y)
		Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_LOGO+FLT_HEADER_SIZE+(ulong)TopOffset, (ulong)ReadSize);
	}
	else{
		Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_LOGO+FLT_HEADER_SIZE, FLT_PRINTLOGO_SIZE);
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadHeader()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�w�b�_�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadHeader(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_HEADER, FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_HEADER+FLT_HEADER_SIZE, FLT_PRINTHEADER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadFooter(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FOOTER, FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FOOTER+FLT_HEADER_SIZE, FLT_PRINTFOOTER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadAccept1Footer()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��t���t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadAcceptFooter(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_ACCEPTFOOTER, FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_ACCEPTFOOTER+FLT_HEADER_SIZE, FLT_ACCEPTFOOTER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Restore()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���X�g�A���s
 *| param	: param_tbl - �ŐV�̃p�����[�^���e�[�u��				<IN>
 *|			  param_tbl_wk - ���[�N�p�p�����[�^���e�[�u��			<WORK>
 *| 		  area_tbl - �ŐV�̃o�b�N�A�b�v�f�[�^���e�[�u��		<IN>
 *|			  area_tbl_wk - ���[�N�p�o�b�N�A�b�v�f�[�^���e�[�u��	<WORK>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_Restore(const t_AppServ_ParamInfoTbl *param_tbl, 
					t_AppServ_ParamInfoTbl *param_tbl_wk,
					const t_AppServ_AreaInfoTbl *area_tbl, 
					t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong	 map_version, verup_flag, ret;
	t_FltReq dummy;
	
/* ���X�g�A�Ώۃf�[�^�̃T���`�F�b�N */
	/* ���X�g�A�\�ȃf�[�^�����邩�`�F�b�N */
	if (!flt_GetVersion(&map_version, &verup_flag)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
/* �e�f�[�^�̃��X�g�A */
	/* �p�����[�^ */
	flt_RestoreParam(param_tbl, param_tbl_wk);
	/* �o�b�N�A�b�v�f�[�^ */
	flt_Restore_BackupData(area_tbl, area_tbl_wk, param_tbl);
	if (map_version >= FLASH2_MAP_VERSION_0100) {	/* �o�[�W�����f�[�^�������͎Ԏ��p�����[�^�̈悪���݂��Ȃ�(ver.0) */
	/* �Ԏ��p�����[�^ */
		flt_RestoreLockParam();
	}
	if (verup_flag != 0 || map_version != FLASH2_MAP_VERSION) {
	/* �o�[�W�������ጟ�o�̂��ߐVMAP�Ńo�b�N�A�b�v */
		ret = flt_WriteParam1(param_tbl, param_tbl_wk, FLT_Ctrl_Backup.workbuf, &dummy, FLT_CMP_ALL);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return ret;
		}
		/* �Ԏ��p�����[�^ */
		ret = flt_WriteLockParam1(FLT_Ctrl_Backup.workbuf, &dummy);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return ret;
		}
		/* �o�b�N�A�b�v�f�[�^ */
		ret = flt_WriteBackupData(area_tbl, area_tbl_wk, param_tbl, 0);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return ret;
		}
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_GetVersion()
 *[]----------------------------------------------------------------------[]
 *| summary	: �������}�b�v�o�[�W�����̎擾
 *| param	: mapver - �擾����MAP�o�[�W�����f�[�^					<OUT>
 *| 		  verup_flag - �o�[�W�����A�b�v�t���O					<OUT>
 *| return	: FALSE - ���X�g�A�p�f�[�^�Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_GetVersion(ulong *mapver, ulong *ver_flg)
{
	ulong vers;
	
	// �o�b�N�A�b�v�e�[�u�����̃T���`�F�b�N
	if (!flt_CheckSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE)) {
		return FALSE;
	}
	/* MAP�o�[�W�����擾 */
	Flash2Read((uchar*)&vers, 
// ���s�́u2�v��t_AppServ_AreaInfoTbl->area_num(ushort)
				FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE+2+sizeof(t_AppServ_AreaInfo)*FLT_DTNUM_MAPVER, 
				sizeof(vers));
	/* ���ʃp�����[�^�̈�SUM�`�F�b�N */
	if (!flt_CheckSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
		return FALSE;
	}
	if (vers >= FLASH2_MAP_VERSION_0100) {	/* �o�[�W�����f�[�^�������͎Ԏ��p�����[�^�̈悪���݂��Ȃ�(ver.0) */
	/* �Ԏ��p�����[�^�̈�SUM�`�F�b�N */
		if (!flt_CheckSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX)) {
			return FALSE;
		}
	}
	
	*mapver = vers;
	/* �o�[�W�����A�b�v�t���O�擾 */
	Flash2Read((uchar*)ver_flg, 
// ���s�́u2�v��t_AppServ_AreaInfoTbl->area_num(ushort)
				FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE+2+sizeof(t_AppServ_AreaInfo)*FLT_DTNUM_VERUPFLG, 
				sizeof(*ver_flg));
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Restore_BackupData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v�f�[�^�̃��X�g�A
 *| param	: area_tbl - ����v���O�����̃o�b�N�A�b�v�f�[�^���e�[�u��		<IN>
 *|			  area_tbl_wk - �t���b�V����̃o�b�N�A�b�v�f�[�^���e�[�u��	<WORK>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_Restore_BackupData(const t_AppServ_AreaInfoTbl *area_tbl, 
							t_AppServ_AreaInfoTbl *area_tbl_wk, 
							const t_AppServ_ParamInfoTbl *param_tbl)
{
	extern	void	Flash2Read_WpOff(uchar *dst, ulong src_addr, ulong size);

	ulong	datasize;
	ushort	section_num, section;
	ulong	src_addr, src_addr2;
	t_AppServ_SectInfo sectinfo;

/* �t���b�V����̃o�b�N�A�b�v�f�[�^���e�[�u���ǂݍ��� */
	flt_ReadAreaTbl(area_tbl_wk);
	
	// �o�b�N�A�b�v�f�[�^�̃��X�g�A
	AppServ_Restore_BackupData(area_tbl, area_tbl_wk);
	if( AppServ_PParam_Copy == ON ){						// FLASH �� RAM �����Ұ���߰�L���H
/* �ʃp�����[�^�e�[�u���̃��X�g�A */
		src_addr = FLT_BACKUP_SECTOR26 + FLT_HEADER_SIZE;
		src_addr2 = FLT_BACKUP_SECTOR27 + FLT_HEADER_SIZE;
// MH810103(s) �c�x�Ə�Ԃ����X�g�A����Ȃ��s��C��
//// MH810103 GG119202(S) ���ݑ䐔�����X�g�A����Ȃ�
//		if (bk_log_ver == 1) {
//			// LOG_VERSION=1(GG1162)���烊�X�g�A����ꍇ��
//			// �ʃp�����[�^�̃o�b�N�A�b�v�Z�N�^������Ă���̂�
//			// LOG_VERSION=1�̂Ƃ��̃o�b�N�A�b�v�Z�N�^���烊�X�g�A����
//			src_addr = (FLT_BACKUP_SECTOR26 + (FLT_BACKUP_SECT_SIZE*4)) + FLT_HEADER_SIZE;
//			src_addr2 = (FLT_BACKUP_SECTOR27 + (FLT_BACKUP_SECT_SIZE*4)) + FLT_HEADER_SIZE;
//		}
// MH810103 GG119202(E) ���ݑ䐔�����X�g�A����Ȃ�
// MH810103(e) �c�x�Ə�Ԃ����X�g�A����Ȃ��s��C��
		Flash2Read((uchar*)&section_num, src_addr, sizeof(section_num));
		if (section_num != 0xFFFF) {
			src_addr += sizeof(section_num);
			for (section = 0; section < section_num; section++) {
				Flash2Read((uchar*)&sectinfo, src_addr, sizeof(sectinfo));
				/* �f�[�^�T�C�Y�̌���(���Ȃ��ق��ɍ��킹��) */
				datasize = param_tbl->psection[section].item_num;
				if (datasize > sectinfo.item_num) {
					datasize = sectinfo.item_num;
				}
				datasize *= FLT_PARAM_DATA_SIZE;
				/* RAM�ւ̃f�[�^�]�� */
				Flash2Read_WpOff(param_tbl->psection[section].address, 
					src_addr2,
					datasize);
				src_addr += sizeof(sectinfo);
				src_addr2 += datasize;
			}
			prm_oiban_chk();
		}
		else {
			/* �ʃp�����[�^�f�[�^����(ver0.0�̃������}�b�v) */
		}
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParamSector()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^�Z�N�^��������
 *| param	: address - �����ݐ�A�h���X
 *|			  data - �������ރf�[�^
 *|			  size - �������ރT�C�Y
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParamSector(ulong address, uchar *data, ulong size)
{
	ulong ret;
	ulong n;

	address &= 0xFFFFF000UL;			// �A�h���X���Z�N�^�擪�ɕ␳
	while (size) {
		if (size > FROM_SECTOR_SIZE) {
			n = FROM_SECTOR_SIZE;
		} else {
			n = size;
		}
		ret = flt_WriteSector(address, data, n);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return ret;
		}
		address += n;
		data += n;
		size -= n;
	}

	return _FLT_MkRtnVal(FLT_NORMAL, FLT_PARAM_WRITTEN);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteSector()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Z�N�^��������
 *| param	: address - �����ݐ�A�h���X
 *|			  data - �������ރf�[�^
 *|			  size - �������ރT�C�Y
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteSector(ulong address, uchar *data, ulong size)
{
	ulong ret;

/* �Z�N�^�C���[�X */
	ret = Flash2EraseSector(address);
	if (ret != FROM2_NORMAL) {
		return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
	}
/* �f�[�^�������� */
	ret = flt_Write(address, data, size);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		Flash2EraseSector(address);
		return ret;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WritePrintDataSector()
 *[]----------------------------------------------------------------------[]
 *| summary	: �v�����g�f�[�^(���S�A�w�b�_�A�t�b�^)��������
 *| param	: data - �������ރf�[�^
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WritePrintDataSector(uchar *data)
{
	ulong ret;
	
/* �Z�N�^�C���[�X */
	ret = flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
	if (ret != FLT_NORMAL) {
		return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
	}

	// �܂Ƃ߂ď������݂���
	ret = flt_Write(FLT_PRINTDATA_SECTOR, data, FLT_PRINTDATA_SECT_SIZE*FLT_PRINTDATA_SECTORMAX);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
		return ret;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Write()
 *[]----------------------------------------------------------------------[]
 *| summary	: �T���t���̃f�[�^���t���b�V����������
 *| param	: address - �����ݐ�A�h���X
 *|			  data - �������ރf�[�^
 *|			  size - �������ރT�C�Y
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_Write(ulong address, uchar *data, ulong size)
{
	ulong ret;

	ret = Flash2Write(address, data, size);
	if (ret != FROM2_NORMAL) {
		return _FLT_MkRtnVal(FLT_WRITE_ERR, ret);
	}

	return _FLT_MkRtnVal2(FLT_NORMAL);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_SetSum()
 *[]----------------------------------------------------------------------[]
 *| summary	: �T���쐬
 *| param	: buf - �����Ώۗ̈�
 *|			  size - �̈�T�C�Y
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_SetSum(uchar *buf, ulong size)
{
	uchar sum = flt_CalcSum(&buf[FLT_HEADER_SIZE], size - FLT_HEADER_SIZE);
	
	buf[0] = sum;
	buf[1] = (uchar)~buf[0];
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CalcSum()
 *[]----------------------------------------------------------------------[]
 *| summary	: �T���v�Z(sumcal()�̃T�C�Y������short�Ȃ̂�ulong�ł����O�ŗp��)
 *| param	: data - �T���Z�o�Ώۗ̈�			<IN>
 *|			  size - �������ރT�C�Y
 *| return	: �T��
 *[]----------------------------------------------------------------------[]*/
uchar	flt_CalcSum(uchar *data, ulong cnt)
{
	uchar sum = 0;
	
	while (cnt-- > 0) {
		sum += *data++;
	}
	
	return sum;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParamTbl()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^���e�[�u���̃t�H�[�}�b�g�ϊ�(�˃t���b�V���t�H�[�}�b�g[=�Z�N�V���������̂݃f�[�^����]�`��)
 *| param	: buf - �R�s�[��o�b�t�@
 *|			  param_tbl - �p�����[�^���e�[�u��			<IN>
 *| return	: �ϊ���̃p�����[�^���e�[�u���T�C�Y
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParamTbl(uchar *buf, const t_AppServ_ParamInfoTbl *param_tbl)
{
	ushort	section;
	ulong	size;
	ulong	ofs;	/* �Z�N�^�擪����̃I�t�Z�b�g�A�h���X */
	t_AppServ_SectInfo	w;
	
	size	= 0;
	ofs		= flt_ParamTblSize(param_tbl) + FLT_HEADER_SIZE;
	
	/* ���ʃp�����[�^�Z�N�V������ */
	memcpy(buf, &param_tbl->csection_num, sizeof(param_tbl->csection_num));
	size += sizeof(param_tbl->csection_num);
	/* �ʃp�����[�^�Z�N�V������ */
	memset(&buf[size], 0, sizeof(param_tbl->psection_num));
	size += sizeof(param_tbl->psection_num);
	/* ���ʃp�����[�^�Z�N�V������� */
	for (section = 0; section < param_tbl->csection_num; section++) {
		w = param_tbl->csection[section];
		w.address = (void*)ofs;
		memcpy(&buf[size], &w, sizeof(w));
		ofs += (param_tbl->csection[section].item_num * FLT_PARAM_DATA_SIZE);
		size += sizeof(param_tbl->csection[0]);
	}
	
	return size;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadParamTbl()
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V������p�����[�^���e�[�u����ǂݍ���
 *| param	: param_tbl_wk - �R�s�[��o�b�t�@				<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_ReadParamTbl(t_AppServ_ParamInfoTbl *param_tbl_wk)
{
	flt_ReadParamTblFrom(param_tbl_wk, FLT_PARAM_SECTOR);
}

void	flt_ReadParamTblFrom(t_AppServ_ParamInfoTbl *param_tbl_wk, ulong sect_addr)
{
	ulong	src_addr;
	ushort	section;
	
	src_addr = sect_addr + FLT_HEADER_SIZE;
	
	/* ���ʃp�����[�^�Z�N�V������ */
	Flash2Read((uchar*)&param_tbl_wk->csection_num, src_addr, sizeof(param_tbl_wk->csection_num));
	src_addr += sizeof(param_tbl_wk->csection_num);
	/* �ʃp�����[�^�Z�N�V������ */
	Flash2Read((uchar*)&param_tbl_wk->psection_num, src_addr, sizeof(param_tbl_wk->psection_num));
	src_addr += sizeof(param_tbl_wk->psection_num);
	/* ���ʃp�����[�^�Z�N�V������� */
	for (section = 0; section < param_tbl_wk->csection_num; section++) {
		Flash2Read((uchar*)&param_tbl_wk->csection[section], src_addr, sizeof(param_tbl_wk->csection[0]));
		src_addr += sizeof(param_tbl_wk->csection[0]);
	}
	/* �ʃp�����[�^�Z�N�V������� */
	for (section = 0; section < param_tbl_wk->psection_num; section++) {
		Flash2Read((uchar*)&param_tbl_wk->psection[section], src_addr, sizeof(param_tbl_wk->psection[0]));
		src_addr += sizeof(param_tbl_wk->psection[0]);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ParamCopy()
 *[]----------------------------------------------------------------------[]
 *| summary	: param_tbl�̏������ɐݒ�p�����[�^�f�[�^��dst�������A�h���X�ɃR�s�[����
 *| param	: dst - �ݒ�p�����[�^�f�[�^�̃R�s�[��
 *|			  param_tbl - �p�����[�^���e�[�u��
 *| return	: TRUE - �R�s�[����
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_ParamCopy(uchar *dst, const t_AppServ_ParamInfoTbl *param_tbl)
{
	ushort	section;
	ulong	copysize;
	
	/* ���ʃp�����[�^ */
	for (section = 0; section < param_tbl->csection_num; section++) {
		copysize = param_tbl->csection[section].item_num * FLT_PARAM_DATA_SIZE;
		if (copysize > 0) {
			memcpy(dst, param_tbl->csection[section].address, copysize);
			dst += copysize;
		}
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ParamTblSize()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^���e�[�u���̃t���b�V����ł̃T�C�Y���擾����
 *| param	: param_tbl - �p�����[�^���e�[�u��
 *| return	: �p�����[�^���e�[�u���̃t���b�V����ł̃T�C�Y
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ParamTblSize(const t_AppServ_ParamInfoTbl *param_tbl)
{
	ulong	tbl_size;
	ushort	section;
	
	tbl_size = sizeof(param_tbl->csection_num) + sizeof(param_tbl->psection_num);
	
	for (section = 0; section < param_tbl->csection_num; section++) {
		tbl_size += sizeof(param_tbl->csection[0]);
	}
	return tbl_size;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteAreaTbl()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�N�A�b�v�f�[�^���e�[�u�����t���b�V���ɏ�������
 *|				(�t���b�V���t�H�[�}�b�g[=�G���A�����̂݃f�[�^����]�`���ŏ�������)
 *| param	: area_tbl - �o�b�N�A�b�v�f�[�^���e�[�u��			<IN>
 *| return	: �t���b�V����̃o�b�N�A�b�v�f�[�^���e�[�u���T�C�Y
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteAreaTbl(const t_AppServ_AreaInfoTbl *area_tbl, ulong ver_flg)
{
	ushort	area;
	ulong	dst;
	ulong	ret;
	t_AppServ_AreaInfo	w;
	
	memset(&FLASH_WRITE_BUFFER[0] ,0xFF ,sizeof(FLASH_WRITE_BUFFER));
	dst = FLT_HEADER_SIZE;
	memcpy(&FLASH_WRITE_BUFFER[dst],(uchar*)&area_tbl->area_num,sizeof(area_tbl->area_num));
	dst += sizeof(area_tbl->area_num);
// �e�G���A���Ƃ̏��̏�������
	for (area = 0; area < area_tbl->area_num; area++) {
		w = area_tbl->area[area];
		if (area == OPE_DTNUM_LOCKINFO) {
			w.size = 0;
		}
		else if (area == FLT_DTNUM_MAPVER) {
			w.address = (void*)FLASH2_MAP_VERSION;
			w.size = 0;
		}
		else if (area == FLT_DTNUM_VERUPFLG) {
			w.address = (void*)ver_flg;
			w.size = 0;
		}
		memcpy(&FLASH_WRITE_BUFFER[dst],(uchar*)&w,sizeof(t_AppServ_AreaInfo));
		dst += sizeof(w);
	}
// �Ώۃf�[�^FROM�ۑ��̈���̏�������
	for (area = 0; area < area_tbl->area_num; area++) {
		memcpy(&FLASH_WRITE_BUFFER[dst],(uchar*)&area_tbl->from_area[area],sizeof(t_AppServ_FromInfo));
		dst += sizeof(t_AppServ_FromInfo);
	}
	flt_SetSum(&FLASH_WRITE_BUFFER[0],FLT_BACKUP_SECT_SIZE);
	ret = Flash2Write(FLT_BACKUP_SECTOR0, &FLASH_WRITE_BUFFER[0], FLT_BACKUP_SECT_SIZE);
	if (ret != FROM2_NORMAL) {
		return _FLT_MkRtnVal(FLT_WRITE_ERR, ret);
	}
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadAreaTbl()
 *[]----------------------------------------------------------------------[]
 *| summary	: �t���b�V������o�b�N�A�b�v�f�[�^���e�[�u����ǂݍ���
 *| param	: param_tbl_wk - �R�s�[��o�b�t�@				<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_ReadAreaTbl(t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong	src_addr;
	ushort	area;
	
	src_addr = FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE;
	
	/* �G���A�� */
	Flash2Read((uchar*)&area_tbl_wk->area_num, src_addr, sizeof(area_tbl_wk->area_num));
	src_addr += sizeof(area_tbl_wk->area_num);
	/* �e�G���A�̏�� */
	for (area = 0; area < area_tbl_wk->area_num; area++) {
		Flash2Read((uchar*)&area_tbl_wk->area[area], src_addr, sizeof(area_tbl_wk->area[0]));
		src_addr += sizeof(area_tbl_wk->area[0]);
	}
	for (area = 0; area < area_tbl_wk->area_num; area++) {
		Flash2Read((uchar*)&area_tbl_wk->from_area[area], src_addr, sizeof(area_tbl_wk->from_area[0]));
		src_addr += sizeof(area_tbl_wk->from_area[0]);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_EraseSectors()
 *[]----------------------------------------------------------------------[]
 *| summary	: �A�����������Z�N�^�C���[�X
 *| param	: top - �Z�N�^�擪
 *|			  sector_size - �Z�N�^�T�C�Y
 *|			  sector_num - �Z�N�^��
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_EraseSectors(ulong top, ulong sector_size, ulong sector_num)
{
	ulong i, ret;
	
	for (i = 0; i < sector_num; i++) {
		ret = Flash2EraseSector(top);
		if (ret != FROM2_NORMAL) {
			return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
		}
		top += sector_size;
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_LockFlash()
 *[]----------------------------------------------------------------------[]
 *| summary	: kind�Ŏw�肳�ꂽ�̈�����b�N����
 *|			  wait_flag!=0�ł���΁A�K�����b�N���Ă��烊�^�[������
 *| param	: req_tbl - �v���Ǘ��e�[�u��				<IN/OUT>
 *|			  kind - �v����̈�(FLT_REQTBL_XXXX)
 *|			  req - �v�����(FLT_REQ_WRITE/FLT_REQ_READ)
 *| return	: TRUE - ���b�N�ł���
 *|-----------------------------------------------------------------------[]
 *| remark	: �{�֐��ł̓��b�N���tskchg���R�[�����Ȃ��̂ŁA�֐����^�[�����_�ł́A
 *|			  �t���b�V���^�X�N�͂��̓�����n�߂Ă��Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_LockFlash(t_FltReq *req_tbl, uchar kind, uchar req, uchar flag)
{
	unsigned long	startTime;
	unsigned long	errdata;
	BOOL			ret = FALSE;
	
	if (flt_CheckBusy(req_tbl, kind, req, flag)) {
		req_tbl[kind].req = req;			/* lock! */
		return TRUE;
	}

	// ���O�f�[�^�̏ꍇ�͈�莞�ԃ��g���C����
	//	if(kind >= (uchar)FLT_REQTBL_LOG && kind < (uchar)FLT_REQTBL_PARAM1) {	// �Ώۂ̓��O�f�[�^(���[�j���O���o��)
	if(kind < FLT_REQTBL_PARAM1) {								// �Ώۂ̓��O�f�[�^
		ret = TRUE;												// ��UTRUE�ɕύX
		startTime = LifeTimGet();
		while(!flt_CheckBusy(req_tbl, kind, req, flag)) {		// ���b�N���g���C
			taskchg( IDLETSKNO );								// task switch (a little bit wait)
			if( 9000 <= LifePastTimGet(startTime) ){			// 90sec(�����Đ����̃Z�}�t�H�҂����ԂƓ���)�o���Ă����b�N�ł��Ȃ��ꍇ�͂�����߂�
				errdata = kind;
				Err_work.Errinf = 2;
				Err_work.ErrBinDat = errdata;
				err_chk2(ERRMDL_MAIN, ERR_MAIN_LOG_WRITE_LOCK, 2, 2, 1, &errdata);	// ���O�����ݎ��̃��b�N���s(����/����) �G���[���=�v����̈�
				ret = FALSE;
				break;
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_UnlockFlash()
 *[]----------------------------------------------------------------------[]
 *| summary	: kind�Ŏw�肳�ꂽ�̈�̃��b�N����������
 *| param	: kind - �v����̈�(FLT_REQTBL_XXXX)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_UnlockFlash(t_FltReq *req_tbl, uchar kind)
{
	if (kind == FLT_REQTBL_PARAM1 || kind == FLT_REQTBL_LOCKPARAM1) {
		if (req_tbl[kind].req == FLT_REQ_WRITE) {
			return;		/* �������ݒ��ɍēx�v�����s��ꂽ�̂ōď����݂ɍs�� */
		}
	}
	req_tbl[kind].req = FLT_REQ_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CheckBusy()
 *[]----------------------------------------------------------------------[]
 *| summary	: kind�Ŏw�肳�ꂽ�̈悪�A�N�Z�X�\���ǂ������`�F�b�N����
 *| param	: req_tbl - �v���Ǘ��e�[�u��				<IN>
 *|			  kind - �v����̈�(FLT_REQTBL_XXXX)
 *|			  req - �v�����(FLT_REQ_WRITE/FLT_REQ_READ)
 *| return	: TRUE - �A�N�Z�X�\
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckBusy(const t_FltReq *req_tbl, uchar kind, uchar req, uchar flag)
{
	int		i;
	BOOL	ret = FALSE;

	if( kind < FLT_REQTBL_LOG+LOG_STRAGEN ){
		/* �����n���h���I�[�v���`�n���h���N���[�Y�܂�LOG�����������N����Ȃ����Ƃ�ۏ� */
		if (req_tbl[kind].req == FLT_REQ_IDLE && req_tbl[FLT_REQTBL_BACKUP].req == FLT_REQ_IDLE) {
			ret = TRUE;
		}
	}
	else if (kind == FLT_REQTBL_PARAM2 && req == FLT_REQ_READ) {
		if (req_tbl[FLT_REQTBL_BACKUP].req == FLT_REQ_IDLE
			&& req_tbl[FLT_REQTBL_RESTORE].req == FLT_REQ_IDLE
			&& req_tbl[FLT_REQTBL_PARAM1].req == FLT_REQ_IDLE
			&& req_tbl[FLT_REQTBL_PARAM2].req == FLT_REQ_IDLE) {
			ret = TRUE;
		}
	}
	else {
		if ((flag & FLT_EXCLUSIVE) == 0) {
	/* FLASH�^�X�N�ł̎��s��v���̃P�[�X */
			if (kind == FLT_REQTBL_PARAM1 || kind == FLT_REQTBL_LOCKPARAM1) {
				ret = TRUE;		/* �K���󂯕t����v�� */
			}
			else {
				if (req_tbl[kind].req == FLT_REQ_IDLE) {
					if (kind == FLT_REQTBL_BACKUP) {
						/* �����n���h���I�[�v���`�n���h���N���[�Y�܂�LOG�����������N����Ȃ����Ƃ�ۏ� */
						for(i=FLT_REQTBL_LOG; i<=FLT_REQTBL_LOG+LOG_STRAGEN; i++){
							if (req_tbl[i].req != FLT_REQ_READ ){
								ret = TRUE;
							}
						}
					}
					else {
						ret = TRUE;
					}
				}
			}
		}
		else {
	/* API�Ăяo�����R���e�L�X�g�ł̃^�X�N�r�����s��v���̃P�[�X */
			switch (kind) {
			case FLT_REQTBL_BACKUP:
			case FLT_REQTBL_RESTORE:
			case FLT_REQTBL_BRLOGSZ:
				// ���O�ȊO��IDLE�ł��邩���`�F�b�N����
				i = FLT_REQTBL_PARAM1;
				break;
			default:
				// �S�Ă̗v����IDLE�ł��邩���`�F�b�N����
				i = FLT_REQTBL_LOG;
				break;
			}
			for ( ; i < FLT_REQTBL_MAX; i++) {
				if (req_tbl[i].req != FLT_REQ_IDLE) {
					break;
				}
			}
			if (i >= FLT_REQTBL_MAX) {
				ret = TRUE;
			}
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_LogErase2()
 *[]----------------------------------------------------------------------[]
 *| summary	: RAM��̎w�肳�ꂽLOG�G���A�̏������s���B
 *| param	: kind  eLog���
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	FLT_LogErase2( unsigned char Kind )
{
	ushort	i;

	memset( &LOG_DAT[Kind], 0, sizeof(struct log_record) );
	// �j�A�t���֘A�����Đݒ肷��
	for(i = 0; i < eLOG_TARGET_MAX; ++i) {
		LOG_DAT[Kind].nearFull[i].NearFullMaximum = Ope_Log_GetNearFullCount(Kind);
		LOG_DAT[Kind].nearFull[i].NearFullMinimum = 0;
		LOG_DAT[Kind].nearFull[i].NearFullStatus = RAU_NEARFULL_CANCEL;
		LOG_DAT[Kind].nearFull[i].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
	}
	// ���O�����������̂�RAU�^�X�N�Ƀj�A�t���i�����j�`�F�b�N��v������
	if(RAUdata_CanSendTableData(Kind)) {
		RAU_RequestCheckNearfull();
	}
	
	/* FlashROM�� LOG����(�Ǘ�)�f�[�^(RAM)�̃N���A */
	if(Kind < LOG_STRAGEN){// FROM�Ɋ��蓖�Ă��Ă��郍�O�̂�LOG����(�Ǘ�)�f�[�^���N���A����
		memset(&FLT_Ctrl_NotBackup.data_info.log_mgr[Kind], 0, sizeof(t_FltLogMgr));
		FLT_Ctrl_NotBackup.data_info.log_mgr[Kind].sectmax = from2_map[Kind].num;
		FLT_Ctrl_NotBackup.data_info.log_mgr[Kind].record_size = from2_map[Kind].size;
		FLT_Ctrl_NotBackup.data_info.log_mgr[Kind].recordnum_max = LOG_SECORNUM(Kind);
		for( i = 0; i < FLT_Ctrl_NotBackup.data_info.log_mgr[Kind].sectmax; i++ ){
			FLT_Ctrl_NotBackup.data_info.log_mgr[Kind].sectinfo[i].address = i*(LogDatMax[Kind][1]+FLT_LOGOFS_RECORD) + from2_map[Kind].top;
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_Restore_FLAPDT()
 *[]----------------------------------------------------------------------[]
 *| summary	: FlashROM���Ƀo�b�N�A�b�v����Ă��� FLAPDT �f�[�^�̃��X�g�A
 *| param	: none
 *| return	: 0x00XXXXXX - ����I��
 *|           0x06XXXXXX - FLT_NODATA (FlashROM���Ƀf�[�^�Ȃ�)
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_Restore_FLAPDT( void )
{
	ushort	area;
	ulong	datasize;
	const t_AppServ_AreaInfoTbl *area_tbl = &FLT_Ctrl_NotBackup.area_tbl;
	t_AppServ_AreaInfoTbl *area_tbl_wk    = &FLT_Ctrl_NotBackup.area_tbl_wk;


	_flt_DisableTaskChg();		// �^�X�N�ؑւȂ��� Flash access���s

	// �e�[�u�����̃T���`�F�b�N
	if (!flt_CheckSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE)) {
		_flt_EnableTaskChg();
		return _FLT_MkRtnVal2(FLT_NODATA);
	}

	/* �t���b�V����̃o�b�N�A�b�v�f�[�^���e�[�u���ǂݍ��� */
	flt_ReadAreaTbl(area_tbl_wk);

	/* �o�b�N�A�b�v�f�[�^�̃��X�g�A */
	area = OPE_DTNUM_FLAPDT;

	/* �f�[�^�T�C�Y�̌���(���Ȃ��ق��ɍ��킹��) */
	datasize = area_tbl->area[area].size;
	if (datasize > area_tbl_wk->area[area].size) {
		datasize = area_tbl_wk->area[area].size;
	}

	/* RAM�ւ̃f�[�^�]�� */
	Flash2Read(area_tbl->area[area].address, 
			area_tbl_wk->from_area[area].from_address,
			datasize);

	_flt_EnableTaskChg();
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: Flt_Comp_ComParaSize_FRom_to_SRam()
 *[]----------------------------------------------------------------------[]
 *| summary	: FlashROM����SRAM���̋������Ұ��ر�̾���ݐ��Ʊ��ѐ����r����B
 *|           �����̊֐��� FLT_init() �����Call���邱�ƁB
 *| param	: none
 *| return	: 0 = FlashROM���ɋ������Ұ��ر�Ȃ�
 *|           1 = FlashROM��SRAM�ł͓�������ݐ��Ʊ��ѐ�
 *|           2 = FlashROM��SRAM�ł;���ݐ� ���� ���ѐ����قȂ�
 *[]----------------------------------------------------------------------[]*/
uchar	FLT_Comp_ComParaSize_FRom_to_SRam( void )
{
	t_AppServ_ParamInfoTbl *pSRAM_param_tbl;			// SRAM�����Ұ����ð���
	t_AppServ_ParamInfoTbl *pFLASH_param_tbl;			// Flash�������Ұ����ð���
	ushort	section, section_max;
	uchar	ret = 1;									// ����ݐ��C���ѐ����ɓ���

	_flt_DisableTaskChg();								// ����֖ؑ���

	/* Flash����e�[�u���ǂݍ��� */
	pFLASH_param_tbl = &FLT_Ctrl_NotBackup.param_tbl_wk;
	flt_ReadParamTbl( pFLASH_param_tbl );

	pSRAM_param_tbl = &FLT_Ctrl_Backup.param_tbl;

	/* ����ݐ���r */
	if (pSRAM_param_tbl->csection_num != pFLASH_param_tbl->csection_num) {
		ret = 2;										// ����ݐ����Ⴄ
		goto FLT_Comp_ComParaSize_FRom_to_SRam_10;
	}

	/* ����ݖ� ���ѐ���r */
	section_max = pSRAM_param_tbl->csection_num;		// ����ݐ�get

	for (section = 0; section < section_max; section++) {
		if (pSRAM_param_tbl->csection[section].item_num != pFLASH_param_tbl->csection[section].item_num) {
			ret = 2;									// ����ݐ����Ⴄ
			goto FLT_Comp_ComParaSize_FRom_to_SRam_10;
		}
	}

FLT_Comp_ComParaSize_FRom_to_SRam_10:
	_flt_EnableTaskChg();								// ����ؑւȂ�����
	return	ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�������擾�i����p�j
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadSyomei(char *buf)
{
	ulong ret;
	
	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadSyomei(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�������擾�i����p�j
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadSyomei(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_SYOMEI, FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_SYOMEI+FLT_HEADER_SIZE, FLT_PRINTSYOMEI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadKamei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�����X�擾�i����p�j
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadKamei(char *buf)
{
	ulong ret;
	
	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadKamei(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadKamei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�����X�擾�i����p�j
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadKamei(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_KAMEI, FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_KAMEI+FLT_HEADER_SIZE, FLT_PRINTKAMEI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}


/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�������������݁i�󎚗p�j
 *| param	: image - �������f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� �^�X�N�������O���ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteSyomei(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_SYOMEI, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.syomei.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteSyomei(&FLT_Ctrl_Backup.params.syomei,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_SYOMEI]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_SYOMEI);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_SYOMEI);
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�������������݁i�󎚗p�j
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteSyomei(const t_FltParam_Syomei *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);

	/* ���S�A�w�b�_�A�t�b�^�A�������A�����X�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */

	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_SYOMEI+FLT_HEADER_SIZE], params->image, FLT_PRINTSYOMEI_SIZE);

	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_SYOMEI], FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE);
	/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
	/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteKamei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�����X�������݁i�󎚗p�j
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteKamei(const t_FltParam_Kamei *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�A�������A�����X�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */

	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_KAMEI+FLT_HEADER_SIZE], params->image, FLT_PRINTKAMEI_SIZE);

	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_KAMEI], FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE);
	/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
	/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteKamei()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�����X�������݁i�󎚗p�j
 *| param	: image - �����X�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� �^�X�N�������O���ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteKamei(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_KAMEI, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.kamei.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteKamei(&FLT_Ctrl_Backup.params.kamei,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_KAMEI]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_KAMEI);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_KAMEI);
	}
	
	return ret;
}

// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�c���`�s�R�}���h�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadEdyAt(char *buf, uchar flags)
{
	ulong ret;
	
	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadEdyAt(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�c���`�s�R�}���h�擾�i�{�����j
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadEdyAt(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EDYAT, FLT_HEADER_SIZE + FLT_EDYATCMD_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EDYAT+FLT_HEADER_SIZE, FLT_EDYATCMD_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�c���`�s�R�}���h��������
 *| param	: image - �d�c���`�s�R�}���h�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� �^�X�N�������O���ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteEdyAt(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EDYAT, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.edyat.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteEdyAt(&FLT_Ctrl_Backup.params.edyat,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EDYAT]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EDYAT);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_EDYAT);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�c���`�s�R�}���h�������݁i�t���b�V���^�X�N���g��Ȃ������݁j
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteEdyAt(const t_FltParam_EdyAt *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_EDYAT+FLT_HEADER_SIZE], params->image, FLT_EDYATCMD_SIZE);

	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_EDYAT], FLT_HEADER_SIZE+FLT_EDYATCMD_SIZE);

/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {

/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {

/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	return ret;
}
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#endif
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���[�U��`���ǂݏo��
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadUserDef(char *buf)
{
	ulong ret;
	
	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadUserDef(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���[�U��`���ǂݏo��
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
static
ulong	flt_ReadUserDef(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_USERDEF, FLT_HEADER_SIZE+FLT_USERDEF_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_USERDEF+FLT_HEADER_SIZE, FLT_USERDEF_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���[�U��`��񏑂�����
 *| param	: image - �����X�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� �^�X�N�������O���ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteUserDef(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_USERDEF, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.userdef.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteUserDef(&FLT_Ctrl_Backup.params.userdef,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_USERDEF]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_USERDEF);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_USERDEF);
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���[�U��`��񏑂�����
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
static
ulong	flt_WriteUserDef(const t_FltParam_UserDef *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�A�������A�����X�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */

	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_USERDEF+FLT_HEADER_SIZE], params->image, FLT_USERDEF_SIZE);

	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_USERDEF], FLT_HEADER_SIZE+FLT_USERDEF_SIZE);
	/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
	/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}
/*--------------------------------------------------------------------------*/
/*		FUNCTIONs for FTP													*/
/*--------------------------------------------------------------------------*/
#define	isFlash1(a)		(a >= FROM1_ADDRESS)

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_SetSum()
 *[]----------------------------------------------------------------------[]
 *| summary	: �T���쐬
 *| param	: buf - �����Ώۗ̈�
 *|			  size - �̈�T�C�Y
 *| return	: none
 *| NOTE    : flt_SetSum()�Ɠ��������A���atchdog�΍��ǉ��ł���悤�P�w�݂��Ă���
 *[]----------------------------------------------------------------------[]*/
void	FLT_SetSum(uchar *buf, ulong size)
{
	flt_SetSum(buf, size);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadEtc()
 *[]----------------------------------------------------------------------[]
 *| summary	: ETC�f�[�^(=FLT_PRINTDATA_SECTOR)�ǂݏo��
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadEtc(char *buf)
{
	/* �{�������s */
	_flt_DisableTaskChg();
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR, _ETC_SIZE_);
	_flt_EnableTaskChg();
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteEtc()
 *[]----------------------------------------------------------------------[]
 *| summary	: ETC�f�[�^(=FLT_PRINTDATA_SECTOR)��������
 *| param	: buf 							<IN>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteEtc(char *buf)
{
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_ETC, FLT_REQ_WRITE, FLT_NOT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.etc.image = buf;

	/* �������݊����͊O�ő҂� */
	return _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_ETC);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteEtc()
 *[]----------------------------------------------------------------------[]
 *| summary	: ETC�f�[�^(=FLT_PRINTDATA_SECTOR)��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
static
ulong	flt_WriteEtc(const t_FltParam_Etc *params, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* �������ݎ��s */
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
	ret = flt_WritePrintDataSector((uchar*)params->image);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* �x���t�@�C */
		if (Flash2Cmp(FLT_PRINTDATA_SECTOR, (uchar*)params->image, _ETC_SIZE_) != 0) {
			flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
			ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
		}
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteImage()
 *[]----------------------------------------------------------------------[]
 *| summary	: �C���[�W�f�[�^��������
 *| param	: buf - �������݃f�[�^				<IN>
 *|			  len -	�f�[�^��					<IN>
 *|			  flash	-	�������ݐ�				<IN>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteImage(char *buf, ulong len, ulong flash, int erase)
{
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_IMAGE, FLT_REQ_WRITE, FLT_NOT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.image.image = buf;
	FLT_Ctrl_Backup.params.image.len = len;
	FLT_Ctrl_Backup.params.image.flash = flash;
	FLT_Ctrl_Backup.params.image.erase = erase;

	/* �������݊����͊O�ő҂� */
	return _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_IMAGE);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteImage()
 *[]----------------------------------------------------------------------[]
 *| summary	: �C���[�W�f�[�^��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
static
ulong	flt_WriteImage(const t_FltParam_Image *params, t_FltReq *req_tbl)
{
	ulong	ret;
	uchar	exec;
	ulong	n;
	ulong	size = 	params->len;
	ulong	address = params->flash;
	uchar	*data = (uchar*)params->image;

	n = (size/FLT_PROGRAM_SECT_SIZE);
	if( !n ){
		n = 1;
	}
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
	if( DOWNLOADING() ){											// ���u�_�E�����[�h��
		if( remotedl_exec_info_get() == FLASH_WRITE_BEFORE ){		// �O���񂪏����ݑO
			exec = FLASH_FIRST_WRITE_START;							// ���񏑍��݂Ƃ���
		}else{
			exec = FLASH_WRITE_START;								// ���������݂Ƃ���
		}
		remotedl_exec_info_set( exec );								// �����X�e�[�^�X��ێ�
	}

	ret = flt_EraseSectors( address, FLT_PROGRAM_SECT_SIZE, n );

	if( _FLT_RtnKind(ret) != FLT_NORMAL ){
		return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
	}

	ret = flt_Write(address, data, size );
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		flt_EraseSectors( GetWriteHeaderAddress(), FLT_PROGRAM_SUM_VER_SECT_SIZE, FLT_PROGRAM_SUM_VER_SECTORMAX );
		return ret;
	}else{
		if (Flash2Cmp(address, data, size) != 0){
			flt_EraseSectors( GetWriteHeaderAddress(), FLT_PROGRAM_SUM_VER_SECT_SIZE, FLT_PROGRAM_SUM_VER_SECTORMAX );
			return _FLT_MkRtnVal(FLT_VERIFY_ERR, FLT_REQTBL_IMAGE);
		}
	}

	return _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_IMAGE);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_DirectWrite()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ڏ�������
 *| param	: buf - �������݃f�[�^				<IN>
 *|			  len -	�f�[�^��					<IN>
 *|			  flash	-	�������ݐ�				<IN>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_DirectWrite(char *buf, ulong len, ulong flash)
{
	ulong	ret;

	_flt_DisableTaskChg();
	ret = Flash2Write(flash, (uchar*)buf, len);
	_flt_EnableTaskChg();
	return _FLT_MkRtnVal2((ret == FROM2_NORMAL)? FLT_NORMAL : FLT_WRITE_ERR);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteParam3()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ҋ@�ʃp�����[�^��������(�����Ƀo�C�i���C���[�W�t��)
 *| param	: param_image - �p�����[�^�ۑ��C���[�W		<IN>
 *|			  image_size - param_image�̗L���f�[�^��
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteParam3(const char *param_image, size_t image_size, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM3, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.param3.param_image	= param_image;
	FLT_Ctrl_Backup.params.param3.image_size	= flt_CParamDataSize(&FLT_Ctrl_Backup.param_tbl);
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteParam3(&FLT_Ctrl_Backup.params.param3, 
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM3]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM3);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_PARAM3);
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParam3()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ҋ@�ʃp�����[�^��������(�����Ƀo�C�i���C���[�W�t��)
 *| param	: params - API���[�U����̈���				<IN>
 *|			  param_tbl - �p�����[�^���e�[�u��		<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParam3(const t_FltParam_Param2 *params, 
					const t_AppServ_ParamInfoTbl *param_tbl, 
					uchar *wbuf,
					t_FltReq *req_tbl)
{
	ulong len;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	ulong sect_addr;
	if(BootInfo.sw.parm == OPERATE_AREA1) {
		sect_addr = FLT_PARAM_SECTOR_2;
	} else {
		sect_addr = FLT_PARAM_SECTOR_1;
	}

/* �������݃C���[�W�쐬 */
	memset(wbuf, 0xFF, (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX));
	len = FLT_HEADER_SIZE;
	/* �p�����[�^���e�[�u�� */
	len += flt_WriteParamTbl(&wbuf[len], param_tbl);
	if (len + params->image_size > FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
	/* �p�����[�^�f�[�^�{�� */
	memcpy(&wbuf[len], params->param_image, params->image_size);
	/* �T�� */
	flt_SetSum(wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
	ret = flt_WriteParamSector(sect_addr, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
		if (Flash2Cmp(sect_addr, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) != 0) {
			flt_EraseSectors(sect_addr, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
			ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
		}
	}
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadStbyParamTbl()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ҋ@�ʃp�����[�^�e�[�u���Ǐo��
 *| param	: param_tbl - �p�����[�^���e�[�u��		<IN/OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	FLT_ReadStbyParamTbl(t_AppServ_ParamInfoTbl *param_tbl_wk)
{
	ushort	section, section_num;
	ulong sect_addr;
	if(BootInfo.sw.parm == OPERATE_AREA1) {
		sect_addr = FLT_PARAM_SECTOR_2;
	} else {
		sect_addr = FLT_PARAM_SECTOR_1;
	}
	flt_ReadParamTblFrom(param_tbl_wk, sect_addr);
	
/* ���ʃp�����[�^ */
	section_num = param_tbl_wk->csection_num;
	for (section = 0; section < section_num; section++) {
		param_tbl_wk->csection[section].address =
			(void*)((ulong)param_tbl_wk->csection[section].address + FTP_buff);
	}
	Flash2Read( (uchar*)FTP_buff, sect_addr, 65536 );
/* �ʃp�����[�^ */
	// �ȗ�
}

//[]----------------------------------------------------------------------[]
///	@brief			���ZLOG�擾(�擪�̃��R�[�h)
//[]----------------------------------------------------------------------[]
///	@param[out]		handle	: �����n���h��
///	@param[out]		buf		: �ŌÂ̐��ZLOG�f�[�^1�����̃f�[�^
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindFirstEpayLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			���ZLOG�̓o�^���������擾(2���ڈȍ~�̃��R�[�h)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	handle	: �����n���h��
///	@param[out]		buf		: ���ZLOG2���ڈȍ~�̃f�[�^
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindNextEpayLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			���ZLOG�̓o�^�����Ɠd�q�}�l�[��ʂ����擾(�擪�̃��R�[�h)
//[]----------------------------------------------------------------------[]
///	@param[out]		handle		: �����n���h��
///	@param[out]		buf			: �ŌÂ̐��ZLOG�f�[�^1�����̃f�[�^
///	@param[in]		seek_cnt	: �A�N�Z�X������̃V�[�N����
///	@param[in]		offset_num	: �ŌÂ����΂���񌏐�
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindFirstEpayKnd(t_FltLogHandle *handle, char *buf, ushort seek_cnt, ushort offset_num)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			���ZLOG�̓o�^�����Ɠd�q�}�l�[��ʂ����擾(2���ڈȍ~�̃��R�[�h)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	handle		: �����n���h��
///	@param[out]		buf			: ���ZLOG2���ڈȍ~�̃f�[�^
///	@param[in]		seek_num	: �A�N�Z�X������̃V�[�N����
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindNextEpayKnd(t_FltLogHandle *handle, char *buf, ushort seek_cnt)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			FlashROM�����A�N�Z�X�p�V�[�N����
//[]----------------------------------------------------------------------[]
///	@param[out]		handle	: �����n���h��
///	@param[out]		num		: �i�߂��񌏐�
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_SeekSaleLog(t_FltLogHandle *handle, ushort num)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadAzuFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �a��؃t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadAzuFtr(char *buf)
{
	ulong ret;

	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadAzuFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadAzuFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �a��؃t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadAzuFtr(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_AZUFTR, FLT_HEADER_SIZE+FLT_AZUFTR_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_AZUFTR+FLT_HEADER_SIZE, FLT_AZUFTR_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteAzuFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �a��؃t�b�^��������
 *| param	: image - �v�����g�t�b�^�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteAzuFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_AZUFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.azuftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteAzuFtr(&FLT_Ctrl_Backup.params.azuftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_AZUFTR]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_AZUFTR);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_AZUFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteAzuFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �a��؃t�b�^��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteAzuFtr(const t_FltParam_AzuFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_AZUFTR+FLT_HEADER_SIZE], params->image, FLT_AZUFTR_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_AZUFTR], FLT_HEADER_SIZE+FLT_AZUFTR_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}

// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�ߕ����t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadCreKbrFtr(char *buf)
{
	ulong ret;

	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadCreKbrFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�ߕ����t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadCreKbrFtr(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_CREKBRFTR, FLT_HEADER_SIZE+FLT_CREKABARAI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_CREKBRFTR+FLT_HEADER_SIZE, FLT_EPAYKABARAI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�ߕ����t�b�^��������
 *| param	: image - �v�����g�t�b�^�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteCreKbrFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_CREKBRFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.crekbrftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteCreKbrFtr(&FLT_Ctrl_Backup.params.crekbrftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CREKBRFTR]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_CREKBRFTR);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_CREKBRFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �N���W�b�g�ߕ����t�b�^��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteCreKbrFtr(const t_FltParam_CreKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_CREKBRFTR+FLT_HEADER_SIZE], params->image, FLT_CREKABARAI_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_CREKBRFTR], FLT_HEADER_SIZE+FLT_CREKABARAI_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadEpayKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�q�}�l�[�ߕ����t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadEpayKbrFtr(char *buf)
{
	ulong ret;

	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadEpayKbrFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadEpayKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�q�}�l�[�ߕ����t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadEpayKbrFtr(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EPAYKBRFTR, FLT_HEADER_SIZE+FLT_EPAYKABARAI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EPAYKBRFTR+FLT_HEADER_SIZE, FLT_EPAYKABARAI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteEpayKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�q�}�l�[�ߕ����t�b�^��������
 *| param	: image - �v�����g�t�b�^�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteEpayKbrFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EPAYKBRFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.epaykbrftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteEpayKbrFtr(&FLT_Ctrl_Backup.params.epaykbrftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EPAYKBRFTR]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EPAYKBRFTR);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_EPAYKBRFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteEpayKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�q�}�l�[�ߕ����t�b�^��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteEpayKbrFtr(const t_FltParam_EpayKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_EPAYKBRFTR+FLT_HEADER_SIZE], params->image, FLT_EPAYKABARAI_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_EPAYKBRFTR], FLT_HEADER_SIZE+FLT_EPAYKABARAI_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���

// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: ����x���z�t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadFutureFtr(char *buf)
{
	ulong ret;

	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadFutureFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: ����x���z�t�b�^�擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadFutureFtr(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FUTUREFTR, FLT_HEADER_SIZE+FLT_FUTURE_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FUTUREFTR+FLT_HEADER_SIZE, FLT_FUTURE_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: ����x���z�t�b�^��������
 *| param	: image - �v�����g�t�b�^�f�[�^		<IN>
 *|			  flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteFutureFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FUTUREFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.futureftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteFutureFtr(&FLT_Ctrl_Backup.params.futureftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FUTUREFTR]);
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FUTUREFTR);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_FUTUREFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: ����x���z�t�b�^��������
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteFutureFtr(const t_FltParam_FutureFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_FUTUREFTR+FLT_HEADER_SIZE], params->image, FLT_FUTURE_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_FUTUREFTR], FLT_HEADER_SIZE+FLT_FUTURE_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[

//[]----------------------------------------------------------------------[]
///	@brief			sw_erase_sector_exclusive(erase sector of switch area)
//[]----------------------------------------------------------------------[]
///	@param[in]		flash	: target address
///	@return			1/0		: OK(1)/NG(0)
///	@attention		None
///	@author			T.Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010/12/16<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
int	FLT_sw_erase_sector_exclusive(char *buf, ulong len, ulong flash, int erase)
{
	ulong	ret;

	_di();
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_IMAGE, FLT_REQ_WRITE, FLT_EXCLUSIVE)) {
		return 0;	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.image.image = 0;
	FLT_Ctrl_Backup.params.image.len = 0;
	FLT_Ctrl_Backup.params.image.flash = flash;
	FLT_Ctrl_Backup.params.image.erase = 1;

	_flt_DisableTaskChg();
	ret = flt_WriteImage( &FLT_Ctrl_Backup.params.image, &FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_IMAGE] );
	_flt_EnableTaskChg();
	
	flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_IMAGE);
	_ei();

	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return 0;
	}
	return 1;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadBRLOGSZ()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���O�T�C�Y���擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadBRLOGSZ( ushort *buf )
{
	ulong ret;

	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadBRLOGSZ(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadBRLOGSZ()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���O�T�C�Y���擾
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadBRLOGSZ( ushort *buf )
{
	/* �f�[�^�L���̃`�F�b�N */
	if( !flt_CheckSum( FLT_BRLOGSZ_SECTOR, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE ) ){
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read( (uchar*)buf, FLT_BRLOGSZ_SECTOR+FLT_HEADER_SIZE, FLT_BR_LOG_SIZE);

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteBRLOGSZ()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���O�T�C�Y��񏑂�����
 *| param	: flags - API����ݒ�
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� ���^�X�N��r�����邩�ۂ�
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteBRLOGSZ( char *buf, uchar flags )
{
	ulong	ret;

	/* �����Ώۗ̈�̃��b�N */
	if( !flt_LockFlash( FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BRLOGSZ, FLT_REQ_WRITE, flags ) ){
		return _FLT_MkRtnVal2( FLT_BUSY );	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	memcpy( &FLT_Ctrl_Backup.params.brlogsz[0].log_size, buf, FLT_BR_LOG_SIZE );

	if( flags & FLT_EXCLUSIVE ){
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();
		ret = flt_WriteBRLOGSZ( &FLT_Ctrl_Backup.params.brlogsz[0],
								 FLT_Ctrl_Backup.workbuf,
								&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_BRLOGSZ] );
		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash( FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BRLOGSZ );
	}else{
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal( FLT_NORMAL, FLT_REQTBL_BRLOGSZ );
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteBRLOGSZ()
 *[]----------------------------------------------------------------------[]
 *| summary	:���O�T�C�Y��񏑂�����
 *| param	: params - API���[�U����̈���				<IN>
 *|			  wbuf - ��d�ۏؑΉ��̃��[�N�̈�			<WORK>
 *|			  req_tbl - �v����t���					<IN/OUT>
 *| return	: 0x00XXXXXX - ����I��
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteBRLOGSZ(const t_FltParam_BRLOGSZ *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);

/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy( &wbuf[FLT_HEADER_SIZE], &params->log_size, FLT_BR_LOG_SIZE );
	/* SUM���쐬 */
	flt_SetSum( wbuf, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE );
/* �������ݎ��s */
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
	ret = flt_WriteSector( FLT_BRLOGSZ_SECTOR, wbuf, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE);
	if( _FLT_RtnKind(ret) == FLT_NORMAL ){
/* �x���t�@�C */
		if ( Flash2Cmp(FLT_BRLOGSZ_SECTOR, wbuf, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE) != 0 ) {
			Flash2EraseSector( FLT_BRLOGSZ_SECTOR );
			ret = _FLT_MkRtnVal2( FLT_VERIFY_ERR );
		}
	}

	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM��̉����f�[�^�^�p�ʏ�����������
//[]----------------------------------------------------------------------[]
///	@param[in]		�^�p�ʏ�� OPERATE_AREA1�܂���OPERATE_AREA2
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_write_wave_swdata(unsigned short sw)
{
	union {
		unsigned char	uc[2];
		unsigned short	us;
	}u;
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// �o�b�t�@256Byte������������
	u.us = sw;
	swdata_write_buf[0] = u.uc[0];
	swdata_write_buf[1] = u.uc[1];
	flt_WriteSector(FLT_WAVE_SWDATA_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM��̋��ʃp�����[�^�^�p�ʏ�����������
//[]----------------------------------------------------------------------[]
///	@param[in]		�^�p�ʏ�� OPERATE_AREA1�܂���OPERATE_AREA2
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_write_parm_swdata(unsigned short sw)
{
	union {
		unsigned char	uc[2];
		unsigned short	us;
	}u;
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// �o�b�t�@256Byte������������
	u.us = sw;
	swdata_write_buf[0] = u.uc[0];
	swdata_write_buf[1] = u.uc[1];
	flt_WriteSector(FLT_PARM_SWDATA_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);

}
//[]----------------------------------------------------------------------[]
///	@brief			FROM��̉����f�[�^�^�p�ʎ擾����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			�^�p�ʏ��̃f�[�^
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short FLT_get_wave_swdata(void)
{
	union {
		unsigned char	uc[2];
		unsigned short	us;
	}u;

	Flash2Read(&u.uc[0], FLT_WAVE_SWDATA_SECTOR, FLT_WAVE_SWDATA_DATA_SIZE);
	return(u.us);
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM��̋��ʃp�����[�^�^�p�ʎ擾����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			�^�p�ʏ��̃f�[�^
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned short FLT_get_parm_swdata(void)
{
	union {
		unsigned char	uc[2];
		unsigned short	us;
	}u;

	Flash2Read(&u.uc[0], FLT_PARM_SWDATA_SECTOR, FLT_PARM_SWDATA_DATA_SIZE);
	return(u.us);
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM��̎w�肵���^�p�ʂ̉����f�[�^�̃o�[�W�����A�T���A�����O�X�̎擾����
//[]----------------------------------------------------------------------[]
///	@param[in]		0:�^�p��1 1:�^�p��2
///	@param[out]		�f�[�^���i�[���ė~�����o�b�t�@�̃|�C���^
///	@return			void
///	@author			A.iiizumi
///	@attention		�i�[����f�[�^����FLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_read_wave_sum_version(unsigned char sw,unsigned char *data)
{
	if(0 == sw){											// �^�p��1
		Flash2Read(data, FLT_SOUND0_SUM_VER_SECTOR, FLT_SUM_VER_DATA_SIZE);
	} else {												// �^�p��2
		Flash2Read(data, FLT_SOUND1_SUM_VER_SECTOR, FLT_SUM_VER_DATA_SIZE);
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM��̃_�E�����[�h�����v���O�����̃o�[�W�����A�T���A�����O�X�̎擾����
//[]----------------------------------------------------------------------[]
///	@param[out]		�f�[�^���i�[���ė~�����o�b�t�@�̃|�C���^
///	@return			void
///	@author			A.iiizumi
///	@attention		�i�[����f�[�^����FLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_read_program_version(unsigned char *data)
{
	Flash2Read(data, FLT_PROGRAM_SUM_VER_SECTOR, FLT_SUM_VER_DATA_SIZE);
}


//[]----------------------------------------------------------------------[]
///	@brief			FROM��̎w�肵���^�p�ʂ̉����f�[�^�̃o�[�W�����A�T���A�����O�X�̏������ݏ���
//[]----------------------------------------------------------------------[]
///	@param[in]		0:�^�p��1 1:�^�p��2
///	@param[in]		�������ރf�[�^���i�[���Ă���o�b�t�@�̃|�C���^
///	@return			void
///	@author			A.iiizumi
///	@attention		�i�[����f�[�^����FLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_write_wave_sum_version(unsigned char sw,unsigned char *data)
{
	ulong	ret;
	// �A�v�����珑�����ݗp�̃f�[�^�Ƃ��Ď󂯎�镪��FLT_PROGRAM_SUM_VER_DATA_SIZE
	// �ł��邪�AFROM�ɂ�256Byte�P�ʂŏ������ނ̂ł���͂��̊֐����Œ�������
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// �o�b�t�@256Byte������������
	memcpy(swdata_write_buf,data,FLT_SUM_VER_DATA_SIZE);// �T���A�o�[�W�������R�s�[

	if(0 == sw){											// �^�p��1
		ret = flt_WriteSector(FLT_SOUND0_SUM_VER_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
	} else {												// �^�p��2
		ret = flt_WriteSector(FLT_SOUND1_SUM_VER_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
	}
	return _FLT_RtnKind(ret);

}
//[]----------------------------------------------------------------------[]
///	@brief			FROM��̃_�E�����[�h�����v���O�����̃o�[�W�����A�T���A�����O�X�̏������ݏ���
//[]----------------------------------------------------------------------[]
///	@param[in]		�������ރf�[�^���i�[���Ă���o�b�t�@�̃|�C���^
///	@return			void
///	@author			A.iiizumi
///	@attention		�i�[����f�[�^����FLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_write_program_version(unsigned char *data)
{
	ulong	ret;
	// �A�v�����珑�����ݗp�̃f�[�^�Ƃ��Ď󂯎�镪��FLT_PROGRAM_SUM_VER_DATA_SIZE
	// �ł��邪�AFROM�ɂ�256Byte�P�ʂŏ������ނ̂ł���͂��̊֐����Œ�������
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// �o�b�t�@256Byte������������
	memcpy(swdata_write_buf,data,FLT_SUM_VER_DATA_SIZE);// �T���A�o�[�W�������R�s�[
	ret = flt_WriteSector(FLT_PROGRAM_SUM_VER_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
	return _FLT_RtnKind(ret);
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�o�^���d����(FROM�������ݏ���)
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_WriteLog_Pon(void)
{
	t_FltParam_Log *params;
	t_FltLogMgr *logmgr;
	struct log_record	*lp;
	int	i,j;
	ushort	sector;
	ushort	seqno;
	for(i=0; i<LOG_STRAGEN; i++){
		params = &FLT_Ctrl_Backup.params.log[i];
		logmgr = &FLT_Ctrl_NotBackup.data_info.log_mgr[i];
		lp = & LOG_DAT[i];
		// LOG�������ݓr���ł̒�d��ۏ�
		if( FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG+i].req==FLT_REQ_EXEC_WRITE ){
			// �ȉ��̏�����flt_GetWriteLogSector()���s��ɒ�d�������Ƃ��l�����ăZ�b�g����
			sector = FLT_Ctrl_Backup.params.log[i].get_sector;
			seqno = FLT_Ctrl_Backup.params.log[i].get_seqno;
			switch(FLT_Ctrl_Backup.params.log[i].f_recover){
				case 0:// �O������
					FLT_Ctrl_Backup.params.log[i].stat_Fcount = lp->Fcount;
					FLT_Ctrl_Backup.params.log[i].stat_sectnum = logmgr->sectnum;
					FLT_Ctrl_Backup.params.log[i].f_recover = 1;// �����J�n
				case 1:// �����J�n
					// �������ݎ��s
					flt_GetWriteLogSector(logmgr, &sector, &seqno);
					flt_WriteLogData(i, params,logmgr,sector, seqno);
					FLT_Ctrl_Backup.params.log[i].f_recover = 2;// �������ݏI��
				case 2:// �Z�N�^�J�E���g�X�V
					if( lp->Fcount < logmgr->sectmax ){
						if(FLT_Ctrl_Backup.params.log[i].stat_Fcount == lp->Fcount){
							// �X�V���O�Œ�d�����̂ōX�V����
							lp->Fcount++;
						}
						FLT_Ctrl_Backup.params.log[i].f_recover = 3;// ���C�g�|�C���^�X�V
					}
				case 3:// ���C�g�|�C���^�X�V
					logmgr->sectinfo[sector].seqno		= seqno;
					logmgr->sectinfo[sector].record_num	= params->record_num;
					break;
				case 40:// �Z�N�^����MAX�𒴂���P�[�X
					goto point40;
				case 41:// �Z�N�^����MAX�𒴂��Ȃ��P�[�X
					goto point41;
				default:// ���J�o���s�v�Ƃ݂Ȃ�
					goto point_end;
			}
			if (logmgr->sectnum + 1 > logmgr->sectmax) {
point40:
				/* ���ɑS�Z�N�^�Ƀf�[�^���������P�[�X */
				logmgr->secttop = _offset(logmgr->secttop, 1, logmgr->sectmax);
				for(j=0; j<eLOG_TARGET_MAX; j++){
					if( lp->unread[j]>LOG_SECORNUM(j) ){
						lp->unread[j] -= LOG_SECORNUM(j);
					}
					else{
						lp->unread[j] = 0;
					}
				}
			}
			else {
point41:
				if(FLT_Ctrl_Backup.params.log[i].stat_sectnum == logmgr->sectnum){
					// �X�V���O�Œ�d�����̂ōX�V����
					logmgr->sectnum++;
				}
			}
point_end:
			LOG_DAT[i].count[FLT_Ctrl_Backup.params.log[i].sramlog_kind] = 0;// FROM�ɏ�������SRAM�̖ʂ��N���A
			FLT_Ctrl_Backup.req_tbl[i].req = FLT_REQ_IDLE;
		}
		FLT_Ctrl_Backup.params.log[i].f_recover = 0;// FROM�̏������ݗv���������ꍇ�̓��J�o���|�C���g���N���A����
		FLT_Ctrl_Backup.params.log[i].stat_sectnum = 0;
		FLT_Ctrl_Backup.params.log[i].stat_Fcount = 0;
		FLT_Ctrl_Backup.params.log[i].get_sector = 0;
		FLT_Ctrl_Backup.params.log[i].get_seqno = 0;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�̗̈�̏������ݗv����ԃ`�F�b�N(�������ݗv�����ł��邩�ۂ�)
//[]----------------------------------------------------------------------[]
///	@param[in]	kind  �v�����
///	@return		TRUE  �������ݗv���ς�
///				FALSE �������ݗv���Ȃ�
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
BOOL FLT_Check_LogWriteReq(short kind)
{
	BOOL ret = FALSE;
	if(FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG + kind].req == FLT_REQ_EXEC_WRITE){
		ret = TRUE;
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�̗̈�̗v����Ԃ������I�ɃN���A����
//[]----------------------------------------------------------------------[]
///	@param[in]	kind  �v�����
///	@return		�Ȃ�
///				
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_Force_ResetLogWriteReq(short kind)
{
// NOTE:���O�ɑ΂��ă��[�h�܂��̓��C�g�̗v���ɂ�胍�b�N��ԂɂȂ������ɕ��d����
// ���O�������݃��J�o���������ɗv���������I�Ƀ��b�N�������Ȃ���
// flt_CheckBusy()�֐��ŃA�N�Z�X�s�ƂȂ�flt_LockFlash()�֐����甲���o���Ȃ��Ȃ邽��
	FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG + kind].req = FLT_REQ_IDLE;// �v������t
	FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_BACKUP].req = FLT_REQ_IDLE;// �v������t
}
//[]----------------------------------------------------------------------[]
///	@brief		�w�肵���A�h���X�̂P�Z�N�^���N���A����
//[]----------------------------------------------------------------------[]
///	@param[in]	addr  ������̃A�h���X
///	@return		�C���[�X����
//[]----------------------------------------------------------------------[]
///	@author		T.Namioka
///	@date		Create	: 2012/05/25<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_DirectEraseSector(ulong addr)
{
	return Flash2EraseSector( addr );
}
//[]----------------------------------------------------------------------[]
///	@brief		FROM�v����t�t���O�N���A����
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		void
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/07/10<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_req_tbl_clear(void)
{
	int i;
	/* �v���󂯕t���������� */
	for (i = 0; i < FLT_REQTBL_MAX; i++) {
		FLT_Ctrl_Backup.req_tbl[i].req		= FLT_REQ_IDLE;
		FLT_Ctrl_Backup.req_tbl[i].result	= _FLT_MkRtnVal2(FLT_NORMAL);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^��FROM�̈�̃Z�N�^�����擾����
//[]----------------------------------------------------------------------[]
///	@param[in]	id  LOG�f�[�^���
///	@return		�Z�N�^��
///				-1 : LOG�f�[�^��ʕs��
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	FLT_Get_LogSectorCount(short id)
{
	if(id <= eLOG_GTTOTAL) {
		return from2_map[id].num;
	}
	else {
		return (ushort)-1;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		FROM���̃��O�o�[�W�����̃��[�h����
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		FROM���̃��O�o�[�W�����f�[�^0x00�`0xFF(0xFF�̓f�[�^���������Ƃ�����)
///	            �o�[�W����0x00��0xFF�͓����Ӗ�
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/11/26<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar FLT_read_log_version(void)
{
	uchar r_log_ver;
	Flash2Read(&r_log_ver, FLT_LOG_VERSION_SECTOR, FLT_LOG_VERSION_DATA_SIZE);
	return(r_log_ver);
}
//[]----------------------------------------------------------------------[]
///	@brief		FROM���̃��O�o�[�W�����̃��C�g����
//[]----------------------------------------------------------------------[]
///	@param[in]	FROM�̃��O�o�[�W����
///	@return		void
///	@author		A.iiizumi
///	@attention	
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/11/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
//void FLT_write_log_version(uchar log_ver)
void FLT_write_log_version(uchar log_ver,uchar chg_info_ver)
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
{
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// �o�b�t�@256Byte������������
	swdata_write_buf[0] = log_ver;// �擪1Byte�ڂɊi�[����
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	// 256�P�ʂ�Write����邽�߁AFLT_LOG_VERSION_DATA_SIZE�͕ς��Ȃ�
	swdata_write_buf[1] = chg_info_ver;// �擪2Byte�ڂɊi�[����
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	flt_WriteSector(FLT_LOG_VERSION_SECTOR, swdata_write_buf, FLT_LOG_VERSION_DATA_SIZE);
}

// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
uchar FLT_read_chg_info_version(void)
{
	uchar ver[2];
	Flash2Read(ver, FLT_LOG_VERSION_SECTOR, sizeof(ver));
	return(ver[1]);
}
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���


// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
//[]----------------------------------------------------------------------[]
///	@brief		�W�v���O�̍폜����
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		1:�폜�K�v�A0:�폜�s�v
///	@author		Y.Tanizaki
///	@attention	
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/06/28<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar FLT_check_syukei_log_delete(void)
{
	uchar i;

	// �Ǘ����ɕύX���Ȃ���ΏW�v���O�������Ȃ�
	if( FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].sectmax != from2_map[eLOG_TTOTAL].num ||
		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].record_size != from2_map[eLOG_TTOTAL].size ||
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].recordnum_max != LOG_SECORNUM(eLOG_TTOTAL) ||
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].sectmax != from2_map[eLOG_LCKTTL].num ||
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].record_size != from2_map[eLOG_LCKTTL].size ||
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].recordnum_max != LOG_SECORNUM(eLOG_LCKTTL) ) {
		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].recordnum_max != LOG_SECORNUM(eLOG_TTOTAL) ) {
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		return 1;
	}
	for( i = 0; i < FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].sectmax; i++ ){
		if(FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].sectinfo[i].address != (i*(LogDatMax[eLOG_TTOTAL][1]+FLT_LOGOFS_RECORD) + from2_map[eLOG_TTOTAL].top)) {
			return 1;
		}
	}
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	for( i = 0; i < FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].sectmax; i++ ){
//		if(FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].sectinfo[i].address != (i*(LogDatMax[eLOG_LCKTTL][1]+FLT_LOGOFS_RECORD) + from2_map[eLOG_LCKTTL].top)) {
//			return 1;
//		}
//	}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
	return 0;
}
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�(�W�v���O���\�Ȍ���c���悤�ɂ���)
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			��Q�A���[�t�b�^�擾
//[]----------------------------------------------------------------------[]
///	@param[out]		buf
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	FLT_ReadEmgFooter(char *buf)
{
	ulong ret;

	/* �{�������s */
	_flt_DisableTaskChg();
	ret = flt_ReadEmgFooter(buf);
	_flt_EnableTaskChg();

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			��Q�A���[�t�b�^�擾
//[]----------------------------------------------------------------------[]
///	@param[out]		buf : 
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	flt_ReadEmgFooter(char *buf)
{
	/* �f�[�^�L���̃`�F�b�N */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EMGFOOTER, FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* �f�[�^�ǂݍ��� */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EMGFOOTER+FLT_HEADER_SIZE, FLT_PRINTEMGFOOTER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

//[]----------------------------------------------------------------------[]
///	@brief			��Q�A���[�t�b�^��������
//[]----------------------------------------------------------------------[]
///	@param[in]		image	: �v�����g�t�b�^�f�[�^
///	@param[in]		flags	: API����ݒ�<br>
///							FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	�� �^�X�N�������O���ۂ�
///	@return			0x00XXXXXX - ����I��
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	FLT_WriteEmgFooter(const char *image, uchar flags)
{
	ulong ret;
	
	/* �����Ώۗ̈�̃��b�N */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EMGFOOTER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ���b�N�ł��Ȃ����� */
	}
	/* �p�����[�^��ۑ� */
	FLT_Ctrl_Backup.params.emgfooter.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* �t���b�V���^�X�N���g��Ȃ��ŏ������� */
		_flt_DisableTaskChg();

		ret = flt_WriteEmgFooter(&FLT_Ctrl_Backup.params.emgfooter,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EMGFOOTER]);

		_flt_EnableTaskChg();
		/* �����Ώۗ̈�̃��b�N���� */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EMGFOOTER);
	}
	else {
	/* �������݊����͊O�ő҂� */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_EMGFOOTER);
	}

	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			��Q�A���[�v�����g�t�b�^��������
//[]----------------------------------------------------------------------[]
///	@param[in]		params	: API���[�U����̈���
///	@param[in]		wbuf	: ��d�ۏؑΉ��̃��[�N�̈�
///	@param[in/our]	req_tbl	: �v����t���
///	@return			0x00XXXXXX	: ����I��
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	flt_WriteEmgFooter(const t_FltParam_EmgFooter *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ���S�A�w�b�_�A�t�b�^�̈���܂Ƃ߂ēǂݏo�� */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* �������݃C���[�W�쐬 */
	/* �������݃f�[�^���R�s�[ */
	memcpy(&wbuf[FLT_PRINTOFS_EMGFOOTER+FLT_HEADER_SIZE], params->image, FLT_PRINTEMGFOOTER_SIZE);
	/* SUM���쐬 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_EMGFOOTER], FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE);
/* �������ރf�[�^�Ɣ�r */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* �������ݎ��s */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* �x���t�@�C */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				Flash2EraseSector(FLT_PRINTDATA_SECTOR);
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}

	return ret;
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
