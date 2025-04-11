/*[]----------------------------------------------------------------------[]
 *| 外部FROM2管理モジュール
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
/* データサイズ */

#define	FLT_PARAM_DATA_SIZE			sizeof(long)

#define	FLT_LOGOFS_RECORD_NUM		2
#define	FLT_LOGOFS_SEQNO			4
#define	FLT_LOGOFS_RECORD			6	/* 先頭レコード位置 */

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
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
#define	FLT_PRINTOFS_CREKBRFTR			_offsetof(_PRNSECT_IMG,crekbrftr)
#define	FLT_PRINTOFS_EPAYKBRFTR			_offsetof(_PRNSECT_IMG,epaykbrftr)
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
#define	FLT_PRINTOFS_FUTUREFTR			_offsetof(_PRNSECT_IMG,futureftr)
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
#define	FLT_PRINTOFS_EMGFOOTER		_offsetof(_PRNSECT_IMG,emgfooter)
// MH810105(E) MH364301 QRコード決済対応
#define	_ETC_SIZE_		((ulong)_offsetof(_PRNSECT_IMG,end))

/* 戻り値作成 */
#define	_FLT_MkRtnVal(result, detail)	(((ulong)(result) << 8*3) + ((ulong)(detail) & 0x00FFFFFF))
#define	_FLT_MkRtnVal2(result)			_FLT_MkRtnVal((result), 0)

/* t_FltReq::req */
enum {
	FLT_REQ_IDLE,		/* 要求未受付 */
	FLT_REQ_WRITE,		/* 書き込み要求受付済み(未実行) */
	FLT_REQ_EXEC_WRITE,	/* 書き込み実行中 */
	FLT_REQ_READ		/* 読み込み実行中 */
};

/* flt_WriteParam1()引数 */
#define	FLT_CMP_ALL			0	/* 共通、個別パラメータとも比較を行う */
#define	FLT_CMP_CPRM_ONLY	1	/* 共通パラメータのみ比較を行う */

#define	FLT_DTNUM_MAPVER		OPE_DTNUM_COMPARA	/* FLASHメモリMAPバージョン位置	 */
#define	FLT_DTNUM_VERUPFLG		OPE_DTNUM_PERPARA	/* バージョンアップフラグ位置 */

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/

extern t_FLT_Ctrl_Backup FLT_Ctrl_Backup;
extern t_FLT_Ctrl_NotBackup FLT_Ctrl_NotBackup;

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

static	ulong	FLT_PARAM_SECTOR;

extern	char	FLT_f_TaskStart;						/* FlashROMタスク起動フラグ */
														/* 0=起動前、1=起動中		*/
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
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY 
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
static ulong	flt_WriteEdyAt(const t_FltParam_EdyAt *params, uchar *wbuf, t_FltReq *req_tbl);
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#endif
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)

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
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
static ulong	flt_ReadCreKbrFtr(char *buf);
static ulong	flt_WriteCreKbrFtr(const t_FltParam_CreKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl);
static ulong	flt_ReadEpayKbrFtr(char *buf);
static ulong	flt_WriteEpayKbrFtr(const t_FltParam_EpayKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl);
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
static ulong	flt_ReadFutureFtr(char *buf);
static ulong	flt_WriteFutureFtr(const t_FltParam_FutureFtr *params, uchar *wbuf, t_FltReq *req_tbl);
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
static ulong	flt_ReadEmgFooter(char *buf);
static ulong	flt_WriteEmgFooter(const t_FltParam_EmgFooter *params, uchar *wbuf, t_FltReq *req_tbl);
// MH810105(E) MH364301 QRコード決済対応
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
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス (RT精算データ対応)
//	{_TYP_DATA,		FLT_LOG1_SECTORMAX,			FLT_LOG1_SECTOR,		sizeof(enter_log)},
	{_TYP_DATA,		FLT_LOG1_SECTORMAX,			FLT_LOG1_SECTOR,		sizeof(RTPay_log)},
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス (RT精算データ対応)
	{_TYP_DATA,		FLT_LOG2_SECTORMAX,			FLT_LOG2_SECTOR,		sizeof(SYUKEI)},
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	{_TYP_DATA,		FLT_LOG3_SECTORMAX,			FLT_LOG3_SECTOR,		sizeof(LCKTTL_LOG)},
	{_TYP_DATA,		FLT_LOG3_SECTORMAX,			FLT_LOG3_SECTOR,		sizeof(RTReceipt_log)},
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	{_TYP_DATA,		FLT_LOG4_SECTORMAX,			FLT_LOG4_SECTOR,		sizeof(Err_log)},
	{_TYP_DATA,		FLT_LOG5_SECTORMAX,			FLT_LOG5_SECTOR,		sizeof(Arm_log)},
	{_TYP_DATA,		FLT_LOG6_SECTORMAX,			FLT_LOG6_SECTOR,		sizeof(Ope_log)},
	{_TYP_DATA,		FLT_LOG7_SECTORMAX,			FLT_LOG7_SECTOR,		sizeof(Mon_log)},
	{_TYP_DATA,		FLT_LOG8_SECTORMAX,			FLT_LOG8_SECTOR,		sizeof(flp_log)},
	{_TYP_DATA,		FLT_LOG9_SECTORMAX,			FLT_LOG9_SECTOR,		sizeof(TURI_KAN)},
	{_TYP_DATA,		FLT_LOG10_SECTORMAX,		FLT_LOG10_SECTOR,		sizeof(ParkCar_log)},
	{_TYP_DATA,		FLT_LOG11_SECTORMAX,		FLT_LOG11_SECTOR,		sizeof(nglog_data)},
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス (QR確定・取消データ対応)
//	{_TYP_DATA,		FLT_LOG12_SECTORMAX,		FLT_LOG12_SECTOR,		sizeof(IoLog_Data)},
	{_TYP_DATA,		FLT_LOG12_SECTORMAX,		FLT_LOG12_SECTOR,		sizeof(DC_QR_log)},
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス (QR確定・取消データ対応)
	{_TYP_DATA,		FLT_LOG13_SECTORMAX,		FLT_LOG13_SECTOR,		sizeof(meisai_log)},
	{_TYP_DATA,		FLT_LOG14_SECTORMAX,		FLT_LOG14_SECTOR,		sizeof(meisai_log)},
	{_TYP_DATA,		FLT_LOG15_SECTORMAX,		FLT_LOG15_SECTOR,		sizeof(meisai_log)},
	{_TYP_DATA,		FLT_LOG16_SECTORMAX,		FLT_LOG16_SECTOR,		sizeof(remote_Change_data)},
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
//	{_TYP_DATA,		FLT_LOG17_SECTORMAX,		FLT_LOG17_SECTOR,		sizeof(LongParking_data)},
	{_TYP_DATA,		FLT_LOG17_SECTORMAX,		FLT_LOG17_SECTOR,		sizeof(LongParking_data_Pweb)},//ParkingWeb用
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
//	{_TYP_DATA,		FLT_LOG18_SECTORMAX,		FLT_LOG18_SECTOR,		sizeof(RismEvent_data)},
	{_TYP_DATA,		FLT_LOG18_SECTORMAX,		FLT_LOG18_SECTOR,		sizeof(DC_LANE_log)},
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
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
 *| summary	: フラッシュモジュール初期化処理
 *| param	: param_written - 停電復帰処理で設定パラメータの書き込みが行われた/行われない	<OUT>
 *|						1 → 行われた
 *|						0 → 行われない
 *| 		  param_ok - 設定パラメータ領域有効/無効										<OUT>
 *|						1 → 有効
 *|						0 → 無効
 *| 		  lockparam_ok - 車室パラメータ領域有効/無効									<OUT>
 *|						1 → 有効
 *|						0 → 無効
 *|			  clr - 1=バックアップRAMデータの強制クリア 0=強制クリアしない(復電処理も無し)
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_init(uchar *param_ok, uchar *lockparam_ok, ulong *ver_flg, char clr)
{
	uchar	i;
	ushort wk_BR_LOG_SIZE[eLOG_MAX];
	uchar	f_LogSzChg = 0;

	/* FROM2ドライバ初期化 */
	Flash2Init();
	Flash1Init();

	// パラメータ領域決定
	if(BootInfo.sw.parm == OPERATE_AREA1){// 面1
		FLT_PARAM_SECTOR = FLT_PARAM_SECTOR_1;
	} else {
		FLT_PARAM_SECTOR = FLT_PARAM_SECTOR_2;
	}
	
	_flt_DisableTaskChg();
	
	if (!clr) {
	/* 停電復帰処理 */
		flt_Fukuden();
	}
	
	/* パラメータ情報テーブル初期化 */
	AppServ_MakeParamInfoTable(&FLT_Ctrl_Backup.param_tbl);
	
	/* バックアップエリア情報テーブル初期化 */
	AppServ_MakeBackupDataInfoTable(&FLT_Ctrl_NotBackup.area_tbl);
	
	/* フラッシュ上のデータチェック＆LOG制御データの作成 */
	flt_ChkFlashData(&FLT_Ctrl_NotBackup.data_info, param_ok, lockparam_ok, ver_flg);
	
	/* 要求受け付けを初期化 */
	for (i = 0; i < FLT_REQTBL_MAX; i++) {
		FLT_Ctrl_Backup.req_tbl[i].req		= FLT_REQ_IDLE;
		FLT_Ctrl_Backup.req_tbl[i].result	= _FLT_MkRtnVal2(FLT_NORMAL);
	}

	// プログラムバージョンアップにて、集計(SYUKEI) or 個別精算(Receipt_data)の形が変わった場合の対策。
	// F1&F3 or F3&F5起動（メモリクリア）時、FlashROM内の両エリアサイズが現在のサイズが異なる場合、
	// FlashROM内の集計/個別精算データも消す。
	// FlashROM内の両エリアサイズは、バックアップデータ内の "BR_LOG_SIZE" から得る。
	// （前提としてバックアップ／リストアの「バックアップ」をこれ以前に行っておく必要がある。）
	// （プログラムバージョンアップ時は、バックアップを行う手順となっている）
	// （この時点ではタスク及びWDTは起動していない）

	if ( clr ) {	// F1&F3 or F3&F5
		for( i = 0; i < eLOG_MAX; i++ ){
			wk_BR_LOG_SIZE[i] = 0;
		}
		if( flt_CheckSum( FLT_BRLOGSZ_SECTOR, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE ) ){	// OK
			FLT_ReadBRLOGSZ( &wk_BR_LOG_SIZE[0] );							// バックアップ時のログサイズ情報をFlashROMから読出し
		}
		for( i = 0; i < eLOG_MAX; i++ ){
			if( wk_BR_LOG_SIZE[i] != LogDatMax[i][0] ){						// サイズ変更チェック
				FLT_LogErase2(i);											// LOGのRAM領域とフラッシュ管理領域クリア
				f_LogSzChg = 1;
			}
		}
		if( f_LogSzChg ){													// サイズ変更あり
			for( i = 0; i < eLOG_MAX ; i++ ){
				BR_LOG_SIZE[i] = LogDatMax[i][0];							// 新ログサイズset
			}
			FLT_WriteBRLOGSZ( (char*)BR_LOG_SIZE, FLT_EXCLUSIVE );			// 新ログサイズ情報書き込み
		}
	}

	_flt_EnableTaskChg();
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteParam1()
 *[]----------------------------------------------------------------------[]
 *| summary	: パラメータ書き込み(引数にバイナリイメージ無し)
 *| param	: flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteParam1(uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM1, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteParam1(&FLT_Ctrl_Backup.param_tbl, 
							&FLT_Ctrl_NotBackup.param_tbl_wk,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM1],
							FLT_CMP_CPRM_ONLY);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM1);
	}
	else {
		/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_PARAM1);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteParam2()
 *[]----------------------------------------------------------------------[]
 *| summary	: パラメータ書き込み(引数にバイナリイメージ付き)
 *| param	: param_image - パラメータ保存イメージ		<IN>
 *|			  image_size - param_imageの有効データ長
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteParam2(const char *param_image, size_t image_size, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.param2.param_image	= param_image;
	FLT_Ctrl_Backup.params.param2.image_size	= flt_CParamDataSize(&FLT_Ctrl_Backup.param_tbl);
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteParam2(&FLT_Ctrl_Backup.params.param2, 
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM2]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_PARAM2);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG書き込み
 *| param	: id - ログ種別
 *|			  records - 精算LOGレコード先頭		<IN>
 *|			  record_num - records内の精算LOGレコード件数(1～FLT_SALELOG_RECORDNUM_MAX)
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *|			  kind - 4Kbyte2面のどちらの情報を書きたいか
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong FLT_WriteLog(short id, const char *records, short record_num, uchar flags, short kind)
{
	ulong	ret;
	uchar	reqid = (uchar)(FLT_REQTBL_LOG+id);

	if( record_num==0 ){		// 正常とする
		return _FLT_MkRtnVal2(FLT_NORMAL);
	}
	if( record_num>LOG_SECORNUM(id) ){
	// 書ける上限まで書き込み
	// ※現仕様上、FLT_LOG_RECORDNUM_MAX=LOG_CNTなのでありえない
		record_num = LOG_SECORNUM(id);
	}

	/* 処理対象領域のロック */
	if( !flt_LockFlash(FLT_Ctrl_Backup.req_tbl, reqid, FLT_REQ_WRITE, flags) ){
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.log[id].records = records;
	FLT_Ctrl_Backup.params.log[id].record_num = record_num;
	FLT_Ctrl_Backup.params.log[id].sramlog_kind = kind;		// SRAM上のLOG(4Kbyte2面のどちらの情報を書きたいかを示すデータ)

	memcpy(FLT_Ctrl_Backup.data_info_bk.log_mgr+id,
			FLT_Ctrl_NotBackup.data_info.log_mgr+id,
								sizeof(t_FltLogMgr));		/* 復電処理用にバックアップ */
	FLT_Ctrl_Backup.req_tbl[reqid].req = FLT_REQ_EXEC_WRITE;	/* 停電保証のチェックポイント */

	if( flags&FLT_EXCLUSIVE ){	// フラッシュタスクを使わない
		_flt_DisableTaskChg();
		ret = flt_WriteLog(FLT_Ctrl_Backup.params.log+id, 
						FLT_Ctrl_NotBackup.data_info.log_mgr+id,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG],
							id);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, reqid);
	}
	else {		// 書き込み完了は外で待つ
		ret = _FLT_MkRtnVal(FLT_NORMAL, reqid);
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLogo()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントロゴ書き込み
 *| param	: image - プリントロゴデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteLogo(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOGO, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.logo.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteLogo(&FLT_Ctrl_Backup.params.logo,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOGO);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_LOGO);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteHeader()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントヘッダ書き込み
 *| param	: image - プリントヘッダデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteHeader(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_HEADER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.header.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteHeader(&FLT_Ctrl_Backup.params.header,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_HEADER]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_HEADER);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_HEADER);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントフッタ書き込み
 *| param	: image - プリントフッタデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteFooter(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FOOTER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.footer.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteFooter(&FLT_Ctrl_Backup.params.footer,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FOOTER]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FOOTER);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_FOOTER);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteAcceptFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受付券フッタ書き込み
 *| param	: image - プリントフッタデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteAcceptFooter(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_ACCEPTFOOTER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.acceptfooter.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteAcceptFooter(&FLT_Ctrl_Backup.params.acceptfooter,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ACCEPTFOOTER]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_ACCEPTFOOTER);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_ACCEPTFOOTER);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLockParam1()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室パラメータ書き込み(引数にバイナリイメージ無し)
 *| param	: flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteLockParam1(uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM1, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteLockParam1(FLT_Ctrl_Backup.workbuf, &FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM1]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM1);
	}
	else {
		/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_LOCKPARAM1);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteLockParam2()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室パラメータ書き込み(引数にバイナリイメージ付き)
 *| param	: param_image - パラメータ保存イメージ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteLockParam2(const char *param_image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM2, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.lockparam2.param_image	= param_image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteLockParam2(&FLT_Ctrl_Backup.params.lockparam2, 
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_LOCKPARAM2);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_LOCKPARAM2);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_Backup()
 *[]----------------------------------------------------------------------[]
 *| summary	: バックアップ
 *| param	: sale_records - 精算LOGレコード
 *|			  sale_record_num - 精算LOGレコード件数
 *|			  total_records - 集計LOGレコード
 *|			  total_record_num - 集計LOGレコード件数
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_Backup(ulong ver_flg,uchar flags)
{
	ulong ret;

	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BACKUP, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.bkup.ver_flg = ver_flg;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わない */
		_flt_DisableTaskChg();
		ret = flt_Backup(
						&FLT_Ctrl_Backup.params.bkup,
						&FLT_Ctrl_Backup.param_tbl,
						&FLT_Ctrl_NotBackup.param_tbl_wk,
						&FLT_Ctrl_NotBackup.area_tbl,
						&FLT_Ctrl_NotBackup.area_tbl_wk,
						FLT_Ctrl_Backup.workbuf);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BACKUP);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_BACKUP);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_RestoreParam()
 *[]----------------------------------------------------------------------[]
 *| summary	: パラメータリストア
 *| param	: none
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_RestoreParam(void)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2, FLT_REQ_READ, FLT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	
	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_RestoreParam(&FLT_Ctrl_Backup.param_tbl, &FLT_Ctrl_NotBackup.param_tbl_wk);
	_flt_EnableTaskChg();
	
	/* 処理対象領域のロック解除 */
	flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2);
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_RestoreLockParam()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室パラメータリストア
 *| param	: none
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_RestoreLockParam(void)
{
	ulong ret;
	
	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_RestoreLockParam();
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetSaleLogRecordNum()
 *[]----------------------------------------------------------------------[]
 *| summary	: 精算LOGレコード件数取得
 *| param	: record_num - LOGレコード件数						<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_GetSaleLogRecordNum(long *record_num)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetTotalLogRecordNum()
 *[]----------------------------------------------------------------------[]
 *| summary	: 集計LOGレコード件数取得
 *| param	: record_num - LOGレコード件数						<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_GetTotalLogRecordNum(long *record_num)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindFirstSaleLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: 精算LOG取得(先頭のレコード)
 *| param	: handle - 検索ハンドル						<OUT>
 *|			  buf - 最古の精算LOGデータ1件分のデータ	<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindFirstSaleLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextSaleLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: 精算LOG取得(2件目以降のレコード)
 *| param	: handle - 検索ハンドル						<IN/OUT>
 *|			  buf - 精算LOG2件目以降のデータ			<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextSaleLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextSaleLog_OnlyDate()	検索高速化用
 *[]----------------------------------------------------------------------[]
 *| summary	: 精算LOGの登録時刻だけ取得(2件目以降のレコード)
 *| param	: handle - 検索ハンドル						<IN/OUT>
 *|			  buf - 精算LOG2件目以降のデータ			<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextSaleLog_OnlyDate(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindFirstSaleLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: 先頭（最古）からｎ番目のデータ読出し
 *| param	: num - 0～719 (0=最古)
 *|			  buf - 精算LOGデータ1件分のデータ	<OUT>
 *|					sizeof(Receipt_data)
 *|			  事前にFlashROM内の登録件数を取得し、有効データのみの読出しを
 *|			  行うことを基本とする。
 *| return	: 0x00XXXXXX - 正常終了
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
 *| summary	: 集計LOG取得(先頭のレコード)
 *| param	: handle - 検索ハンドル						<OUT>
 *|			  buf - 最古の集計LOGデータ1件分のデータ	<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindFirstTotalLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextTotalLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: 精算LOG取得(2件目以降のレコード)
 *| param	: handle - 検索ハンドル						<IN/OUT>
 *|			  buf - 精算LOG2件目以降のデータ			<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextTotalLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_FindNextTotalLog_OnlyDate()	検索高速化用
 *[]----------------------------------------------------------------------[]
 *| summary	: 集計LOGの登録時刻だけ取得(2件目以降のレコード)
 *| param	: handle - 検索ハンドル						<IN/OUT>
 *|			  buf - 精算LOG2件目以降のデータ			<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_FindNextTotalLog_OnlyDate(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_TargetTotalLogRead()
 *[]----------------------------------------------------------------------[]
 *| summary	: 先頭（最古）からｎ番目のデータ読出し
 *| param	: num - 0～20 (0=最古)
 *|			  buf - 精算LOGデータ1件分のデータ	<OUT>
 *|					sizeof(Syu_log)
 *|			  事前にFlashROM内の登録件数を取得し、有効データのみの読出しを
 *|			  行うことを基本とする。
 *| return	: 0x00XXXXXX - 正常終了
 *|			  0x05XXXXXX - FLT_BUSY
 *|			  0x06XXXXXX - FLT_NODATA
 *[]----------------------------------------------------------------------[]
 *|	Auther	: Okuda : 2005/08/19
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_TargetTotalLogRead( ushort num, char *buf )
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief		LOG取得(先頭のレコード)
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG種別
///	@param[in/out]	handle	: 検索ハンドル
///	@param[out]	buf			: 最古のLOG1件分のデータ
///	@return					: 0x00XXXXXX - 正常終了
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_FindFirstLog(short id, t_FltLogHandle *handle, char *buf)
{
	ulong ret;

	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, (uchar)(FLT_REQTBL_LOG+id), FLT_REQ_READ, FLT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}

	/* 検索ハンドル初期化 */
	handle->kind		= FLT_SEARCH_LOG + id;
	handle->sectno		= FLT_Ctrl_NotBackup.data_info.log_mgr[id].secttop;
	handle->recordno	= 0;
	/* 本処理実行 */
	ret = flt_ReadLogRecord(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle, buf);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		FLT_CloseLogHandle(handle);
		return ret;
	}
	/* 検索ハンドル更新 */
	flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle);

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG取得(2件目以降のレコード)
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG種別
///	@param[in/out]	handle	: 検索ハンドル
///	@param[out]	buf			: LOG2件目以降のデータ
///	@return					: 0x00XXXXXX - 正常終了
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

	/* 処理対象領域がロックされているかチェック */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG + id].req != FLT_REQ_READ) {
		return _FLT_MkRtnVal2(FLT_NOT_LOCKED);	/* ロックされていない */
	}

	/* 本処理実行 */
	ret = flt_ReadLogRecord(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle, buf);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* 検索ハンドル更新 */
		flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGの登録時刻だけ取得(2件目以降のレコード)
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG種別
///	@param[in/out]	handle	: 検索ハンドル
///	@param[out]	buf			: LOG2件目以降のデータ
///	@return					: 0x00XXXXXX - 正常終了
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

	/* 本処理実行 */
	logmgr = FLT_Ctrl_NotBackup.data_info.log_mgr+id;
	if (logmgr->sectmax <= handle->sectno) {	// イニシャルコーディング
		ret = _FLT_MkRtnVal2(FLT_NODATA);	/* 指定位置にレコード無し */
	}
	else if (handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		ret = _FLT_MkRtnVal2(FLT_NODATA);	/* 指定位置にレコード無し */
	}
	else {
		/* データ読み出し */
		src_addr = logmgr->sectinfo[handle->sectno].address 
				+ FLT_LOGOFS_RECORD
				+ (handle->recordno * logmgr->record_size);
		Flash2Read((uchar*)buf, src_addr, (ulong)25L);	// 登録時刻までget
		ret = _FLT_MkRtnVal2(FLT_NORMAL);
	}

	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* 検索ハンドル更新 */
		flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, handle);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		先頭（最古）からｎ番目のデータ読出し
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG種別
///	@param[in]	num			: 0～20 (0=最古)
///	@param[out]	buf			: LOG1件分のデータ
///	@return					: 0x00XXXXXX - 正常終了
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
	if( num>=recn ){		// 正常とする
		return _FLT_MkRtnVal2(FLT_NORMAL);
	}

	/* 処理対象領域のロック */
	if( !flt_LockFlash(FLT_Ctrl_Backup.req_tbl, reqid, FLT_REQ_READ, FLT_EXCLUSIVE) ){
		return _FLT_MkRtnVal2(FLT_BUSY);		/* ロックできなかった */
	}

	/* 検索ハンドル初期化 */
	handle.kind		= FLT_SEARCH_LOG+id;
	handle.sectno	= FLT_Ctrl_NotBackup.data_info.log_mgr[id].secttop;
	handle.recordno	= 0;

	/* 本処理実行 */
	/* Target位置までSkip */
	for(i=0; i<num; i++){
		ulret = flt_NextLog(FLT_Ctrl_NotBackup.data_info.log_mgr+id, &handle);
		if(_FLT_RtnKind(ulret)!=FLT_NORMAL ){	/* error (Y) */
			FLT_CloseLogHandle(&handle);		/* Handle & Lock release */
			return	ulret;						/* error return */
		}
		WACDOG;
	}

	/* Terget Logデータ読出し */
	ulret = flt_ReadLogRecord(FLT_Ctrl_NotBackup.data_info.log_mgr+id, &handle, buf);

	FLT_CloseLogHandle( &handle );				/* Handle & Lock release */

	return ulret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGレコード件数取得
//[]----------------------------------------------------------------------[]
///	@param[in]	id			: LOG種別
///	@param[out]	record_num	: LOGレコード件数
///	@return					: 0x00XXXXXX - 正常終了
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/14<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_GetLogRecordNum(short id, long *record_num)
{
	/* 処理対象領域のロック */
	if( !flt_LockFlash(FLT_Ctrl_Backup.req_tbl, (uchar)(FLT_REQTBL_LOG+id), FLT_REQ_READ, FLT_EXCLUSIVE) ){
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}

	/* 本処理実行 */
	*record_num = flt_GetLogRecordNum(FLT_Ctrl_NotBackup.data_info.log_mgr+id);

	/* 処理対象領域のロック解除 */
	flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, (uchar)(FLT_REQTBL_LOG+id));

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadLogo()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントロゴ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadLogo(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	ret = flt_ReadLogo(buf, (char)0, 0, 0);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadLogoPart()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントロゴ取得（データ部分読出し）
 *|			  指定オフセット位置(TopOffset)からしていサイズ分(ReadSize)
 *|			  読出しを行い *buf にセットする。
 *| param	: buf 							<OUT>
 *| 		  TopOffset						<IN>
 *| 		  ReadSize						<IN>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadLogoPart(char *buf, ushort TopOffset, ushort ReadSize)
{
	ulong ret;

	/* 本処理実行 */
	ret = flt_ReadLogo(buf, (char)1, TopOffset, ReadSize);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadHeader()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントヘッダ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadHeader(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	ret = flt_ReadHeader(buf);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントフッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadFooter(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	ret = flt_ReadFooter(buf);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadAcceptFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受付券フッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadAcceptFooter(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	ret = flt_ReadAcceptFooter(buf);

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_Restore()
 *[]----------------------------------------------------------------------[]
 *| summary	: リストア実行
 *| param	: flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_Restore(uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_RESTORE, FLT_REQ_READ, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないでリストア*/
		_flt_DisableTaskChg();
		ret = flt_Restore(&FLT_Ctrl_Backup.param_tbl,
						&FLT_Ctrl_NotBackup.param_tbl_wk,
						&FLT_Ctrl_NotBackup.area_tbl,
						&FLT_Ctrl_NotBackup.area_tbl_wk);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_RESTORE);
	}
	else {
	/* リストア完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_RESTORE);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ChkWriteCmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: 書き込み完了チェック
 *| param	: req_kind - 完了をチェックする書き込み種別(書き込みAPIの戻り値)
 *|			  endsts - 書き込み結果			<OUT>
 *|					0x00XXXXXX - 正常終了
 *| return	: 1 - 完了
 *|			  0 - 未完了
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
 *| summary	: LOG検索ハンドルクローズ
 *| param	: handle - LOG検索ハンドル					<IN>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	FLT_CloseLogHandle(t_FltLogHandle *handle)
{
	// 処理対象領域のロック解除
	if( handle->kind<=FLT_SEARCH_LOG+LOG_STRAGEN ){
		if( FLT_Ctrl_Backup.req_tbl[handle->kind].req==FLT_REQ_READ ){
			flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, handle->kind);
		}
	}
	// ハンドル無効化
	handle->kind = FLT_SEARCH_NONE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetProgramVersion()
 *[]----------------------------------------------------------------------[]
 *| summary	: FLASH上に保存されたプログラムバージョンを取得する
 *| param	: ver_flg - 取得したバージョンアップフラグ			<OUT>
 *| return	: 0 - リストア用のデータなし
 *[]----------------------------------------------------------------------[]*/
uchar	FLT_GetVerupFlag(ulong *ver_flg)
{
	ulong dummy;
	
	return flt_GetVersion(&dummy, ver_flg);
}


/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_GetParamTable()
 *[]----------------------------------------------------------------------[]
 *| summary	: パラメータ情報テーブル取得
 *| param	: none
 *| return	: パラメータ情報テーブル
 *[]----------------------------------------------------------------------[]*/
const t_AppServ_ParamInfoTbl	*FLT_GetParamTable(void)
{
	return &FLT_Ctrl_Backup.param_tbl;
}

/*====================================================================================[TASK]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: flatask()
 *[]----------------------------------------------------------------------[]
 *| summary	: フラッシュタスク本体
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flatask(void)
{
	int	i;
	ulong	ret;
	uchar	exec;
	t_FltReq *req_tbl = FLT_Ctrl_Backup.req_tbl;
	
	FLT_f_TaskStart = 1;						/* FlashROMタスク起動中情報セット */

	while (1) {
		taskchg(IDLETSKNO);
		
		exec = FLT_REQTBL_MAX;
		
		if (req_tbl[FLT_REQTBL_BACKUP].req == FLT_REQ_WRITE) {
		/* バックアップ */
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
		/* リストア */
			ret = flt_Restore(&FLT_Ctrl_Backup.param_tbl,
							&FLT_Ctrl_NotBackup.param_tbl_wk,
							&FLT_Ctrl_NotBackup.area_tbl,
							&FLT_Ctrl_NotBackup.area_tbl_wk);
			exec = FLT_REQTBL_RESTORE;
		}
		else if (req_tbl[FLT_REQTBL_PARAM2].req == FLT_REQ_WRITE) {
		/* パラメータ(イメージ作成済み) */
			ret = flt_WriteParam2(&FLT_Ctrl_Backup.params.param2,
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf, 
							&req_tbl[FLT_REQTBL_PARAM2]);
			exec = FLT_REQTBL_PARAM2;
		}
		else if (req_tbl[FLT_REQTBL_PARAM1].req == FLT_REQ_WRITE) {
		/* パラメータ(イメージ無し) */
			ret = flt_WriteParam1(&FLT_Ctrl_Backup.param_tbl, 
							&FLT_Ctrl_NotBackup.param_tbl_wk,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM1],
							FLT_CMP_CPRM_ONLY);
			exec = FLT_REQTBL_PARAM1;
		}
		else if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2].req == FLT_REQ_WRITE) {
		/* 車室パラメータ(イメージ付き) */
			ret = flt_WriteLockParam2(&FLT_Ctrl_Backup.params.lockparam2,
							FLT_Ctrl_Backup.workbuf, 
							&req_tbl[FLT_REQTBL_LOCKPARAM2]);
			exec = FLT_REQTBL_LOCKPARAM2;
		}
		else if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM1].req == FLT_REQ_WRITE) {
		/* 車室パラメータ(イメージ無し) */
			ret = flt_WriteLockParam1(FLT_Ctrl_Backup.workbuf, &FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM1]);
			exec = FLT_REQTBL_LOCKPARAM1;
		}
		else if (req_tbl[FLT_REQTBL_LOGO].req == FLT_REQ_WRITE) {
		/* ロゴ */
			ret = flt_WriteLogo(&FLT_Ctrl_Backup.params.logo, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_LOGO]);
			exec = FLT_REQTBL_LOGO;
		}
		else if (req_tbl[FLT_REQTBL_HEADER].req == FLT_REQ_WRITE) {
		/* ヘッダ */
			ret = flt_WriteHeader(&FLT_Ctrl_Backup.params.header, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_HEADER]);
			exec = FLT_REQTBL_HEADER;
		}
		else if (req_tbl[FLT_REQTBL_FOOTER].req == FLT_REQ_WRITE) {
		/* フッタ */
			ret = flt_WriteFooter(&FLT_Ctrl_Backup.params.footer, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_FOOTER]);
			exec = FLT_REQTBL_FOOTER;
		}
		else if (req_tbl[FLT_REQTBL_ACCEPTFOOTER].req == FLT_REQ_WRITE) {
		/* 受付券フッタ */
			ret = flt_WriteAcceptFooter(&FLT_Ctrl_Backup.params.acceptfooter, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_ACCEPTFOOTER]);
			exec = FLT_REQTBL_ACCEPTFOOTER;
		}
		else if (req_tbl[FLT_REQTBL_SYOMEI].req == FLT_REQ_WRITE) {
		/* 署名欄 */
			ret = flt_WriteSyomei(&FLT_Ctrl_Backup.params.syomei, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_SYOMEI]);
			exec = FLT_REQTBL_SYOMEI;
		}

		else if (req_tbl[FLT_REQTBL_KAMEI].req == FLT_REQ_WRITE) {
		/* 加盟店 */
			ret = flt_WriteKamei(&FLT_Ctrl_Backup.params.kamei, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_KAMEI]);
			exec = FLT_REQTBL_KAMEI;
		}
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
		else if (req_tbl[FLT_REQTBL_EDYAT].req == FLT_REQ_WRITE) {
//		/* ＥｄｙＡＴコマンド */
			ret = flt_WriteEdyAt(&FLT_Ctrl_Backup.params.edyat, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_EDYAT]);
			exec = FLT_REQTBL_EDYAT;
		}
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#endif
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
		else if (req_tbl[FLT_REQTBL_USERDEF].req == FLT_REQ_WRITE) {
		/* ユーザ定義 */
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
		/* 待機面パラメータ(イメージ作成済み) */
			ret = flt_WriteParam3(&FLT_Ctrl_Backup.params.param3,
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf, 
							&req_tbl[FLT_REQTBL_PARAM3]);
			exec = FLT_REQTBL_PARAM3;
		}
		else if (req_tbl[FLT_REQTBL_AZUFTR].req == FLT_REQ_WRITE) {
			/* 預り証フッタ */
			ret = flt_WriteAzuFtr(&FLT_Ctrl_Backup.params.azuftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_AZUFTR]);
			exec = FLT_REQTBL_AZUFTR;
		}
// 仕様変更(S) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
		else if (req_tbl[FLT_REQTBL_CREKBRFTR].req == FLT_REQ_WRITE) {
			/* クレジット過払いフッタ */
			ret = flt_WriteCreKbrFtr(&FLT_Ctrl_Backup.params.crekbrftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_CREKBRFTR]);
			exec = FLT_REQTBL_CREKBRFTR;
		}
		else if (req_tbl[FLT_REQTBL_EPAYKBRFTR].req == FLT_REQ_WRITE) {
			/* 電子マネー過払いフッタ */
			ret = flt_WriteEpayKbrFtr(&FLT_Ctrl_Backup.params.epaykbrftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_EPAYKBRFTR]);
			exec = FLT_REQTBL_EPAYKBRFTR;
		}
// 仕様変更(E) K.Onodera 2016/10/31 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
		else if (req_tbl[FLT_REQTBL_FUTUREFTR].req == FLT_REQ_WRITE) {
			/* 後日支払額フッタ */
			ret = flt_WriteFutureFtr(&FLT_Ctrl_Backup.params.futureftr, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_FUTUREFTR]);
			exec = FLT_REQTBL_FUTUREFTR;
		}
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
		else if (req_tbl[FLT_REQTBL_EMGFOOTER].req == FLT_REQ_WRITE) {
		/* 障害連絡票フッタ */
			ret = flt_WriteEmgFooter(&FLT_Ctrl_Backup.params.emgfooter, FLT_Ctrl_Backup.workbuf, &req_tbl[FLT_REQTBL_EMGFOOTER]);
			exec = FLT_REQTBL_EMGFOOTER;
		}
// MH810105(E) MH364301 QRコード決済対応
		else{
			for(i=FLT_REQTBL_LOG; i<FLT_REQTBL_PARAM1; i++){	// 履歴データ
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
		/* いずれかの処理が行われた */
			/* 実行結果保存 */
			req_tbl[exec].result = ret;
			/* 領域ロック解除 */
			flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, exec);
		}
	}
}

/*====================================================================================[OTHERS]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Fukuden()
 *[]----------------------------------------------------------------------[]
 *| summary	: 復電処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_Fukuden(void)
{
// ログの復電はこれより以前にFLT_WriteLog_Pon()で実施済み
	ulong	ret;

/* プリントロゴ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO].req = FLT_REQ_IDLE;
	}
/* プリントヘッダ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_HEADER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_HEADER].req = FLT_REQ_IDLE;
	}
/* プリントフッタ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FOOTER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FOOTER].req = FLT_REQ_IDLE;
	}
/* 受付券フッタ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ACCEPTFOOTER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ACCEPTFOOTER].req = FLT_REQ_IDLE;
	}
/* プリント署名欄 */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_SYOMEI].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_SYOMEI].req = FLT_REQ_IDLE;
	}
/* プリント加盟店 */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_KAMEI].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_KAMEI].req = FLT_REQ_IDLE;
	}
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EDYAT].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EDYAT].req = FLT_REQ_IDLE;
	}
// ユーザ定義
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_USERDEF].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_USERDEF].req = FLT_REQ_IDLE;
	}
// ETC cache
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ETC].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector((uchar*)FLT_Ctrl_Backup.params.etc.image);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_ETC].req = FLT_REQ_IDLE;
	}
	/* プログラムイメージは復電処理を行わない */
	/* 待機面パラメータイメージは復電処理を行わない */

/* 設定パラメータ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM2].req == FLT_REQ_EXEC_WRITE) {
		ret = flt_WriteParamSector(FLT_PARAM_SECTOR, FLT_Ctrl_Backup.workbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			_flt_DestroyParamRamSum();		/* RAM上のパラメータサムをクリア */
		}
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM2].req = FLT_REQ_IDLE;
	}
/* 車室パラメータ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2].req == FLT_REQ_EXEC_WRITE) {
		ret = flt_WriteParamSector(FLT_LOCKPARAM_SECTOR, FLT_Ctrl_Backup.workbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
			_flt_DestroyLockParamRamSum();		/* RAM上のパラメータサムをクリア */
		}
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOCKPARAM2].req = FLT_REQ_IDLE;
	}
/* Tカードフッタ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_TCARDFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_TCARDFTR].req = FLT_REQ_IDLE;
	}
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CARDDATA].req == FLT_REQ_EXEC_WRITE) {
		flt_WriteSector( FLT_CARDDATA_SECTOR, FLT_Ctrl_Backup.workbuf, FLT_CARDDATA_SECT_SIZE);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CARDDATA].req = FLT_REQ_IDLE;
	}
/* 預り証フッタ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_AZUFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_AZUFTR].req = FLT_REQ_IDLE;
	}
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
/* クレジット過払いフッタ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CREKBRFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CREKBRFTR].req = FLT_REQ_IDLE;
	}
/* 電子マネー過払いフッタ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EPAYKBRFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EPAYKBRFTR].req = FLT_REQ_IDLE;
	}
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
/* 後日支払額フッタ */
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FUTUREFTR].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FUTUREFTR].req = FLT_REQ_IDLE;
	}
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
	if (FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EMGFOOTER].req == FLT_REQ_EXEC_WRITE) {
		flt_WritePrintDataSector(FLT_Ctrl_Backup.workbuf);
		FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EMGFOOTER].req = FLT_REQ_IDLE;
	}
// MH810105(E) MH364301 QRコード決済対応
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ChkFlashData()
 *[]----------------------------------------------------------------------[]
 *| summary	: 各領域のサムをチェックし、データが有効無効を判断する
 *|			  サムNGでブランクでない場合は、イレースを行う
 *| param	: data_info - 各領域の管理データ			<OUT>
 *|			  param_ok - 設定パラメータ領域の有効/無効	<OUT>
 *|						1 → 有効
 *|						0 → 無効
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_ChkFlashData(t_FltDataInfo *data_info, 
						uchar *param_ok, 
						uchar *lockparam_ok, 
						ulong *ver_flg)
{
	uchar restore_ok;
	
/* LOG領域 */
	if( read_rotsw() == 4 ){	// ----- スーパーイニシャライズ(RSW=4)? -----
		flt_ChkLog(data_info->log_mgr);
	}
	
/* バックアップ領域 */
	restore_ok = flt_CheckAreaSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE, 1);// テーブルのチェック
/* 共通パラメータ、個別パラメータ領域 */
	*param_ok = flt_CheckAreaSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
// 待機面についてもチェック
	if(BootInfo.sw.parm == OPERATE_AREA1) {
		flt_CheckAreaSum(FLT_PARAM_SECTOR_2, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
	} else {
		flt_CheckAreaSum(FLT_PARAM_SECTOR_1, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
	}
	restore_ok &= *param_ok;
/* 車室パラメータ領域 */
	*lockparam_ok = flt_CheckAreaSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE, FLT_LOCKPARAM_SECTORMAX);
	restore_ok &= *lockparam_ok;
/* プリンタロゴ、ヘッダ、フッタ領域 */
	flt_ChkPrintData(FLT_Ctrl_Backup.workbuf, &FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOGO]);
	
	if (restore_ok) {
/* バージョンアップフラグ取得 */
		Flash2Read((uchar*)ver_flg, 
// 下行の「2」はt_AppServ_AreaInfoTbl->area_num(ushort)
				FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE+2+sizeof(t_AppServ_AreaInfo)*FLT_DTNUM_VERUPFLG, 
				sizeof(*ver_flg));
	}
	else {
		*ver_flg = 0;	/* バージョンアップ要求なし */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ChkLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: フラッシュ上の精算・集計LOGデータと、その管理データの初期化
 *| param	: salelog_mgr - 精算LOG管理データ
 *| 		  totallog_mgr - 集計LOG管理データ
 *| return	: none
 // iiduka 2012/02/22
 *[]----------------------------------------------------------------------[]*/
//	指定ログのクリアを実施するための関数に変更された。
//	従来機種ではログエリアの非バックアップ管理データを初期化するための関数だったが、
//	このエリアもバックアップされるように変更されている。
void	flt_ChkLog(t_FltLogMgr *log_mgr)
{
	short	i, j;

	// LOG初期化
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
 *| summary	: フラッシュ+RAMのLOGの管理領域の初期化
 *| param	: kind  eLog種別
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
 *| summary	: FLASH上領域のチェックサム照合
 *| param	: top_address - 領域先頭アドレス
 *|			  sectsize - セクタサイズ
 *|			  sectnum - セクタ数
 *| return	: TRUE - サムOK
 *|-----------------------------------------------------------------------[]
 *| remark	: サムNGの領域がブランクでなかったら、イレースを行う
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckAreaSum(ulong top_address, ulong sectsize, ushort sectnum)
{
	BOOL	sum_ok, blank;
	
	sum_ok = flt_CheckData(top_address, sectsize*sectnum, &blank);
	if (!sum_ok && !blank) {
	/* サムNGなのにデータあり */
		flt_EraseSectors(top_address, sectsize, (ulong)sectnum);	/* サムが関係するセクタをイレース */
	}
	
	return sum_ok;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ChkPrintData()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントデータ領域の各データ有無をチェックする
 *|			  サムNGでブランクでない場合は、そのデータのイレースを行う
 *| param	: wbuf - 書き込み用ワークバッファ			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_ChkPrintData(uchar *wbuf, t_FltReq *req_tbl)
{
	BOOL	sum, blank, write;
	
	write = FALSE;
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, _ETC_SIZE_);
	
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_LOGO, 
					FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_LOGO], 0xFF, FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE);			/* データイレース */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_HEADER, 
					FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_HEADER], 0xFF, FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE);		/* データイレース */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FOOTER, 
					FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_FOOTER], 0xFF, FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE);		/* データイレース */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_ACCEPTFOOTER, 
					FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_ACCEPTFOOTER], 0xFF, FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE);		/* データイレース */
		write = TRUE;
	}

	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_SYOMEI, 
					FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_SYOMEI], 0xFF, FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE);		/* データイレース */
		write = TRUE;
	}

	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_KAMEI, 
					FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_KAMEI], 0xFF, FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE);		/* データイレース */
		write = TRUE;
	}

// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EDYAT, 
//					FLT_HEADER_SIZE+FLT_EDYATCMD_SIZE, 
//					&blank);
//	if (!sum && !blank) {
//	/* サムNGなのにデータあり */
//		memset(&wbuf[FLT_PRINTOFS_EDYAT], 0xFF, FLT_HEADER_SIZE+FLT_EDYATCMD_SIZE);			/* データイレース */
//		write = TRUE;
//	}
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
	
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_USERDEF, 
					FLT_HEADER_SIZE+FLT_USERDEF_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_USERDEF], 0xFF, FLT_HEADER_SIZE+FLT_USERDEF_SIZE);		/* データイレース */
		write = TRUE;
	}

	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_TCARDFTR, 
					FLT_HEADER_SIZE+FLT_TCARDFTR_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_TCARDFTR], 0xFF, FLT_HEADER_SIZE+FLT_TCARDFTR_SIZE);		/* データイレース */
		write = TRUE;
	}
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_AZUFTR, 
					FLT_HEADER_SIZE+FLT_AZUFTR_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_AZUFTR], 0xFF, FLT_HEADER_SIZE+FLT_AZUFTR_SIZE);		/* データイレース */
		write = TRUE;
	}
// MH810105(S) MH364301 QRコード決済対応
	sum = flt_CheckData(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EMGFOOTER, 
					FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE, 
					&blank);
	if (!sum && !blank) {
	/* サムNGなのにデータあり */
		memset(&wbuf[FLT_PRINTOFS_EMGFOOTER], 0xFF, FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE);	/* データイレース */
		write = TRUE;
	}
// MH810105(E) MH364301 QRコード決済対応

	if (write) {
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
	/* データ書き込み */
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
 *| summary	: データチェック
 *| param	: top_address - 先頭アドレス
 *|			  size - サイズ
 *|			  blank - TRUE=ブランク						<OUT>
 *| return	: TRUE - サムOK
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckData(ulong top_address, ulong size, BOOL *blank)
{
	*blank = FALSE;
	
	if (!flt_CheckSum(top_address, size)) {
	/* サムがない */
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
 *| summary	: サムチェック
 *| param	: top_address - 先頭アドレス
 *|			  size - サイズ
 *| return	: TRUE - サムOK
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckSum(ulong top_address, ulong size)
{
	uchar	c[FLT_HEADER_SIZE];
	
	/* SUM読み込み */
	Flash2Read(c, top_address, sizeof(c));
	
	if( (c[0]^c[1])!= 0xFF ){	// サムがない
		return FALSE;
	}
	else if (c[0] != Flash2Sum(top_address+FLT_HEADER_SIZE, size-FLT_HEADER_SIZE)) {	// SUM異常
		return FALSE;
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParam1()
 *[]----------------------------------------------------------------------[]
 *| summary	: パラメータ書き込み(引数にバイナリイメージ無し)
 *| param	: param_tbl - パラメータ情報テーブル		<IN>
 *|			  param_tbl_wk - ワーク用パラメータ情報テーブル		<WORK>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *|			  cmp - 書き込み前後での比較方法(FLT_CMP_ALL/FLT_CMP_CPRM_ONLY)
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParam1(const t_AppServ_ParamInfoTbl *param_tbl, 
						t_AppServ_ParamInfoTbl *param_tbl_wk, 
						uchar *wbuf, t_FltReq *req_tbl, uchar cmp_method)
{
	ulong len;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 要求受付のチェックポイント */

	if( prm_invalid_check() ){
		DataSumUpdate(OPE_DTNUM_COMPARA);				/* update parameter sum on ram */
	}
	
/* RAM⇔FLASHの比較 */
	if (flt_CmpParamData(param_tbl, param_tbl_wk, cmp_method) != 0) {
/* 書き込みイメージ作成 */
		memset(wbuf, 0xFF, (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX));
		len = FLT_HEADER_SIZE;
		/* パラメータ情報テーブル */
		len += flt_WriteParamTbl(&wbuf[len], param_tbl);
		if (len + flt_CParamDataSize(param_tbl) > (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
			return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
		}
		/* パラメータデータ本体 */
		flt_ParamCopy(&wbuf[len], param_tbl);
		/* サム */
		flt_SetSum(wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
/* 書き込み実行 */
		ret = flt_WriteParamSector(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	}
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ(最新のデータ本体と比較) */
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
 *| summary	: RAM⇔FLASH パラメータデータ内容の比較
 *| param	: param_tbl - パラメータ情報テーブル		<IN>
 *|			  param_tbl_wk - ワーク用パラメータ情報テーブル		<WORK>
 *|			  cmp - 書き込み前後での比較方法(FLT_CMP_ALL/FLT_CMP_CPRM_ONLY)
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CmpParamData(const t_AppServ_ParamInfoTbl *param_tbl, 
						t_AppServ_ParamInfoTbl *param_tbl_wk, 
						uchar cmp_method)
{
	ushort section;
/* サムのチェック */
	if (!flt_CheckSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
		return 1;
	}
/* フラッシュ上のバックアップ情報テーブルを読み込み */
	flt_ReadParamTbl(param_tbl_wk);
/* 共通パラメータ */
	/* セクション数 */
	if (param_tbl->csection_num != param_tbl_wk->csection_num) {
		return 1;
	}
	for (section = 0; section < param_tbl->csection_num; section++) {
	/* 各セクションの項目数 */
		if (param_tbl->csection[section].item_num != param_tbl_wk->csection[section].item_num) {
			return 1;
		}
	/* 各セクションのデータ内容 */
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
 *| summary	: パラメータ書き込み(引数にバイナリイメージ付き)
 *| param	: params - APIユーザからの引数				<IN>
 *|			  param_tbl - パラメータ情報テーブル		<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParam2(const t_FltParam_Param2 *params, 
					const t_AppServ_ParamInfoTbl *param_tbl, 
					uchar *wbuf,
					t_FltReq *req_tbl)
{
	ulong len;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* 書き込みイメージ作成 */
	memset(wbuf, 0xFF, (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX));
	len = FLT_HEADER_SIZE;
	/* パラメータ情報テーブル */
	len += flt_WriteParamTbl(&wbuf[len], param_tbl);
	if (len + params->image_size > FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
	/* パラメータデータ本体 */
	memcpy(&wbuf[len], params->param_image, params->image_size);
	/* サム */
	flt_SetSum(wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	
/* 書き込み先イメージと比較 */
	if (Flash2Cmp(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WriteParamSector(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
			if (Flash2Cmp(FLT_PARAM_SECTOR, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) != 0) {
				flt_EraseSectors(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE, FLT_PARAM_SECTORMAX);
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
		_flt_DestroyParamRamSum();		/* RAM上のパラメータサムをクリア */
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: ログ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  logmgr - LOG管理データ					<IN/OUT>
 *|			  logmgr_bk - 復電処理用のバックアップ領域	<OUT>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *|			  clr_kind - RAMデータ削除コード
 *| return	: 0x00XXXXXX - 正常終了
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
	params->f_recover = 1;// 処理開始
/* 書込み情報作成 */
	flt_GetWriteLogSector(logmgr, &sector, &seqno);
	params->get_sector = sector;
	params->get_seqno = seqno;
/* 書込み実行 */
	if(logmgr->sectinfo[sector].address >= FLT_WAVE_SWDATA_SECTOR){
		// 音声データ運用面情報の領域以降にアクセスした場合何もしない
		// バックアップバッテリーのジャンパがOPENの状態で起動した時、ログの管理領域が壊れ（logmgr->sectinfo[sector].address）
		// に音声データの領域を指定された時、音声データを破壊しないようにするためあえて正常終了を返す
		return _FLT_MkRtnVal2(FLT_NORMAL);
	}
	ret = flt_WriteLogData(clr_kind, params, logmgr, sector, seqno);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		goto _err;
	}
/* ベリファイ */
	if (flt_CmpLogData(clr_kind, params, logmgr, sector, seqno) != 0) {
	// この場合も書き込みエラー登録する
		ret = FROM2_ADDRESS+logmgr->sectinfo[sector].address;
		FlashRegistErr(ERR_MAIN_FLASH_WRITE_ERR, ret);
		ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
		goto _err;
	}
	params->f_recover = 2;// 書き込み終了
/* LOG制御データ更新 */
	if( lp->Fcount < logmgr->sectmax ){
		lp->Fcount++;
		params->f_recover = 3;// ライトポインタ更新
	}
	logmgr->sectinfo[sector].seqno		= seqno;
	logmgr->sectinfo[sector].record_num	= params->record_num;
	if (logmgr->sectnum + 1 > logmgr->sectmax) {
		params->f_recover = 40;// セクタ数がMAXを超えるケース
		/* 既に全セクタにデータがあったケース */
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
		params->f_recover = 41;// セクタ数がMAXを超えないケース
		logmgr->sectnum++;
	}
/* RAM上のLOGデータクリア */
	// 書き込んだ側のSRAM上のLOG(4Kbyte2面のどちらの情報を書きたいかを示すデータ)のカウントを消去する
	LOG_DAT[clr_kind].count[params->sramlog_kind] = 0;
	params->f_recover = 0;// 処理終了
	return _FLT_MkRtnVal2(FLT_NORMAL);
	
_err:
	/* 書込み/ベリファイに失敗したセクタは無効とする */
	Flash2EraseSector(logmgr->sectinfo[sector].address);
	logmgr->sectinfo[sector].record_num = 0;
	if (logmgr->sectnum >= logmgr->sectmax) {
		logmgr->secttop = _offset(logmgr->secttop, 1, logmgr->sectmax);
		logmgr->sectnum--;
	}
	// 書き込んだ側のSRAM上のLOG(4Kbyte2面のどちらの情報を書きたいかを示すデータ)のカウントを消去する
	LOG_DAT[clr_kind].count[params->sramlog_kind] = 0;
	params->f_recover = 0;// 処理終了
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_GetWriteLogSector()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOGデータ書込み先セクタ情報取得
 *| param	: logmgr - LOG管理データ									<IN>
 *|			  sector - 書き込み先セクタ番号(0～logmgr->sectmax-1)		<OUT>
 *|			  seqno - 書込み先セクタの追い番							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
void	flt_GetWriteLogSector(const t_FltLogMgr *logmgr, ushort *sector, ushort *seqno)
{
	/* 書き込み先セクタ取得 */
	*sector	= _offset(logmgr->secttop, logmgr->sectnum, logmgr->sectmax);
	/* 書き込み先セクタの追い番取得 */
	if (logmgr->sectnum <= 0) {
		*seqno = 0;
	}
	else {
		/* ひとつ前のセクタの追い番+1 */
		*seqno = logmgr->sectinfo[_offset(*sector, logmgr->sectmax-1, logmgr->sectmax)].seqno;
		if( ++(*seqno)>=0xffff ){
			*seqno = 0;
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLogData()
 *[]----------------------------------------------------------------------[]
 *| summary	: ログ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  logmgr - LOG管理データ					<IN/OUT>
 *|			  sector - 書き込み先セクタ番号(0～logmgr->sectmax-1)
 *|			  seqno - 書込み先セクタの追い番
 *| return	: 0x00XXXXXX - 正常終了
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
/* 書込み先セクタイレース */
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
	// レコード
	memcpy(FLASH_WRITE_BUFFER+FLT_LOGOFS_RECORD, params->records, dsize);
	// サム
	FLASH_WRITE_BUFFER[0] = flt_CalcSum(FLASH_WRITE_BUFFER+FLT_HEADER_SIZE, 3+dsize);
	FLASH_WRITE_BUFFER[1] = (uchar)~FLASH_WRITE_BUFFER[0];

/* 書込み */
	ret = Flash2Write(dst, FLASH_WRITE_BUFFER, FLT_LOGOFS_RECORD+dsize);
	if (ret != FROM2_NORMAL) {
		return  _FLT_MkRtnVal(FLT_WRITE_ERR, ret);
	}

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CmpLogData()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOGデータ内容比較(FLASH⇔RAM)
 *| param	: params - APIユーザからの引数				<IN>
 *|			  logmgr - LOG管理データ					<IN/OUT>
 *|			  sector - 書き込みを行ったセクタの番号(0～logmgr->sectmax-1)
 *|			  seqno - 書込みを行ったセクタの追い番
 *| return	: 0=相違無し
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CmpLogData(ushort Lno, const t_FltParam_Log *params, const t_FltLogMgr *logmgr, ushort sector, ushort seqno)
{
	ulong	addr;
	ushort	record_num, i;
	ulong	dsize;

	dsize = LogDatMax[Lno][1]+FLT_LOGOFS_RECORD;

	record_num = params->record_num;

	/* サムのチェック */
	addr = logmgr->sectinfo[sector].address;
	if( !flt_CheckSum(addr, dsize) ){
		return 1;
	}
	addr += FLT_HEADER_SIZE;
	/* レコード数 */
	if (Flash2Cmp(addr, (uchar*)&record_num, 2) != 0) {
		return 1;
	}
	addr+=2;
	/* 追い番 */
	if( Flash2Cmp(addr, (uchar*)&seqno, 2)!=0 ){
		return 1;
	}
	addr += 2;
	/* レコード */
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
 *| summary	: プリントロゴ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLogo(const t_FltParam_Logo *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong size;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
/* 書き込みデータ長をチェック */
	size = get_bmplen((uchar*)params->image);
	if (size <= 0 || FLT_PRINTLOGO_SIZE < size) {
		return FLT_INVALID_SIZE;
	}
/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_LOGO+FLT_HEADER_SIZE], params->image, FLT_PRINTLOGO_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_LOGO], FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
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
 *| summary	: プリントヘッダ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteHeader(const t_FltParam_Header *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_HEADER+FLT_HEADER_SIZE], params->image, FLT_PRINTHEADER_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_HEADER], FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
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
 *| summary	: プリントフッタ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteFooter(const t_FltParam_Footer *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_FOOTER+FLT_HEADER_SIZE], params->image, FLT_PRINTFOOTER_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_FOOTER], FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
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
 *| summary	: 受付券フッタ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteAcceptFooter(const t_FltParam_AcceptFooter *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_ACCEPTFOOTER+FLT_HEADER_SIZE], params->image, FLT_ACCEPTFOOTER_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_ACCEPTFOOTER], FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
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
 *| summary	: 車室パラメータ書き込み(引数にバイナリイメージ無し)
 *| param	: wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLockParam1(uchar *wbuf, t_FltReq *req_tbl)
{
	union {
		ushort	s;
		uchar	c[2];
	}u;
	ulong size;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 要求受付のチェックポイント */
	
/* データ長チェック */
	size = FLT_LKPRMOFS_PARAM + sizeof(LockInfo);
	if (size > FLT_LOCKPARAM_SECT_SIZE) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
/* RAM⇔FLASHの比較 */
	if (flt_CmpLockParamData() != 0) {
/* 書き込みイメージ作成 */
		memset(wbuf, 0xFF, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
		/* 車室数 */
		u.s = LOCK_MAX;
		wbuf[FLT_LKPRMOFS_LOCKMAX]		= u.c[0];
		wbuf[FLT_LKPRMOFS_LOCKMAX+1]	= u.c[1];
		/* 車室毎のデータサイズ */
		u.s = sizeof(t_LockInfo);
		wbuf[FLT_LKPRMOFS_DATASIZE]		= u.c[0];
		wbuf[FLT_LKPRMOFS_DATASIZE+1]	= u.c[1];
		/* 車室パラメータ本体 */
		memcpy(&wbuf[FLT_LKPRMOFS_PARAM], LockInfo, sizeof(LockInfo));
		/* サム */
		flt_SetSum(wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);

/* 書き込み実行 */
		ret = flt_WriteParamSector(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
	}
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ(最新のデータ本体と比較) */
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
 *| summary	: RAM⇔FLASH 車室パラメータデータ内容の比較
 *| param	: none
 *| return	: 0＝相違無し
 *[]----------------------------------------------------------------------[]*/
ulong	flt_CmpLockParamData(void)
{
	ushort	lockmax, datasize;
	
/* サムのチェック */
	if (!flt_CheckSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX)) {
		return 1;
	}
/* フラッシュ上のデータ管理情報を読み込み */
	Flash2Read((uchar*)&lockmax, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_LOCKMAX, sizeof(lockmax));
	Flash2Read((uchar*)&datasize, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_DATASIZE, sizeof(datasize));
/* 車室数 */
	if (lockmax != LOCK_MAX) {
		return 1;
	}
/* 車室毎のデータサイズ */
	if (datasize != sizeof(t_LockInfo)) {
		return 1;
	}
/* 車室パラメータの内容 */
	if (Flash2Cmp(FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_PARAM, (uchar*)LockInfo, sizeof(LockInfo)) != 0) {
		return 1;
	}
	
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteLockParam2()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室パラメータ書き込み(引数にバイナリイメージ付き)
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteLockParam2(const t_FltParam_LockParam2 *params, uchar *wbuf, t_FltReq *req_tbl)
{
	union {
		ushort	s;
		uchar	c[2];
	}u;
	ulong size;
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* データ長チェック */
	size = FLT_LKPRMOFS_PARAM + sizeof(LockInfo);
	if (size > FLT_LOCKPARAM_SECT_SIZE) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
/* 書き込みイメージ作成 */
	memset(wbuf, 0xFF, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
	/* 車室数 */
	u.s = LOCK_MAX;
	wbuf[FLT_LKPRMOFS_LOCKMAX]		= u.c[0];
	wbuf[FLT_LKPRMOFS_LOCKMAX+1]	= u.c[1];
	/* 車室毎のデータサイズ */
	u.s = sizeof(t_LockInfo);
	wbuf[FLT_LKPRMOFS_DATASIZE]		= u.c[0];
	wbuf[FLT_LKPRMOFS_DATASIZE+1]	= u.c[1];
	/* パラメータデータ本体 */
	memcpy(&wbuf[FLT_LKPRMOFS_PARAM], params->param_image, sizeof(LockInfo));
	/* サム */
	flt_SetSum(wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
	
/* 書き込み先イメージと比較 */
	if (Flash2Cmp(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WriteParamSector(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
			if (Flash2Cmp(FLT_LOCKPARAM_SECTOR, wbuf, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX) != 0) {
				flt_EraseSectors(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE, FLT_LOCKPARAM_SECTORMAX);
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
		_flt_DestroyLockParamRamSum();		/* RAM上のパラメータサムをクリア */
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Backup()
 *[]----------------------------------------------------------------------[]
 *| summary	: バックアップ実行
 *| param	: salelog_params - APIユーザからの引数				<IN>
 *|			  totallog_params - APIユーザからの引数				<IN>
 *|			  param_tbl - パラメータ情報テーブル				<IN>
 *|			  area_tbl - バックアップデータ情報テーブル			<IN>
 *|			  area_tbl_wk - ワーク用バックアップ情報テーブル	<WORK>
 *|			  wbuf - ワークバッファ								<WORK>
 *|			  loginfo - LOG管理データ							<IN/OUT>
 *|			  loginfo_wk - LOG管理データの復電用バックアップ	<OUT>
 *|			  req_tbl - 要求受付情報							<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
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

/* 各領域のバックアップ */
	/* 設定パラメータ */
	ret = flt_WriteParam1(param_tbl, param_tbl_wk, wbuf, &dummy, FLT_CMP_ALL);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* 車室パラメータ */
	ret = flt_WriteLockParam1(wbuf, &dummy);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* バックアップデータ */
	ret = flt_WriteBackupData(area_tbl, area_tbl_wk, param_tbl, bkup_params->ver_flg);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteBackupData()
 *[]----------------------------------------------------------------------[]
 *| summary	: バックアップデータ領域書き込み
 *| param	: area_tbl - バックアップデータ情報テーブル				<IN>
 *|			  area_tbl_wk - ワーク用バックアップデータ情報テーブル	<WORK>
 *| return	: 0=相違無し
 *[]----------------------------------------------------------------------[]*/
static ulong	flt_WriteBackupData(const t_AppServ_AreaInfoTbl *area_tbl,
								t_AppServ_AreaInfoTbl *area_tbl_wk,
								const t_AppServ_ParamInfoTbl *param_tbl,
								ulong ver_flg)
{
	ulong ret;
	
	/* dstとsrcの比較 */
	if (flt_CmpBackupData(area_tbl, area_tbl_wk, param_tbl, ver_flg) != 0) {
	/* 書き込み */
		ret = flt_WriteBackupDataSector(area_tbl, param_tbl, ver_flg);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			flt_EraseSectors(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE, FLT_BACKUP_SECTORMAX);
			return ret;
		}
	/* ベリファイ(最新のデータ本体と比較) */
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
 *| summary	: バックアップデータの比較(RAM⇔FLASH)
 *| param	: area_tbl - バックアップデータ情報テーブル				<IN>
 *|			  area_tbl_wk - ワーク用バックアップデータ情報テーブル	<WORK>
 *| return	: 0=相違無し
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
	
	/* サムのチェック */
	// テーブル情報のサムチェック
	if (!flt_CheckSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE)) {
		return 1;
	}
	/* フラッシュ上のバックアップ情報テーブルを読み込み */
	flt_ReadAreaTbl(area_tbl_wk);
	/* エリア数の比較 */
	if (area_tbl->area_num != area_tbl_wk->area_num) {
		return 1;
	}
	for (area = 0; area < area_tbl->area_num; area++) {
	/* 各エリアのサイズの比較 */
		if (area == OPE_DTNUM_LOCKINFO) {
			if (area_tbl_wk->area[area].size == 0) {
				continue;
			}
		}
		if (area_tbl->area[area].size != area_tbl_wk->area[area].size) {
			return 1;
		}
	/* 各エリアのデータの比較 */
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
	/* フラッシュ上の個別パラメータ情報テーブル読込み */
	src_addr = FLT_BACKUP_SECTOR26 + FLT_HEADER_SIZE;
	src_addr2 = FLT_BACKUP_SECTOR27 + FLT_HEADER_SIZE;
	/* 個別パラメータセクション数の比較 */
	Flash2Read((uchar*)&section_num, src_addr, sizeof(section_num));
	if (section_num == 0xFFFF || section_num != param_tbl->psection_num) {
		return 1;
	}
	src_addr += sizeof(section_num);
	for (section = 0; section < section_num; section++) {
		Flash2Read((uchar*)&sectinfo, src_addr, sizeof(sectinfo));
	/* セクション上データ数の比較 */
		if (sectinfo.item_num != param_tbl->psection[section].item_num) {
			return 1;
		}
	/* 個別パラメータセクション情報の比較 */
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
 *| summary	: バックアップデータ領域の書き込み
 *| param	: area_tbl - バックアップデータ情報テーブル		<IN>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteBackupDataSector(const t_AppServ_AreaInfoTbl *area_tbl, const t_AppServ_ParamInfoTbl *param_tbl, ulong ver_flg)
{
	ulong	ret;
	
	/* バックアップデータ領域イレース */
	ret = flt_EraseSectors(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE, FLT_BACKUP_SECTORMAX);
	if( ret != FLT_NORMAL ){
		return ret;
	}
	/* テーブル書き込み */
	ret = flt_WriteAreaTbl(area_tbl, ver_flg);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* バックアップデータ書き込み */
	ret = flt_WriteAreaData(area_tbl);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* 個別パラメータテーブル書込み */
	ret = flt_WritePParamTbl(param_tbl, FLT_BACKUP_SECTOR26);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	/* 個別パラメータ書込み */
	ret = flt_WritePParamData(param_tbl, FLT_BACKUP_SECTOR27);
	if (_FLT_RtnKind(ret) != FLT_NORMAL) {
		return ret;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WritePParamTbl()
 *[]----------------------------------------------------------------------[]
 *| summary	: 個別パラメータ情報テーブルのバックアップ領域への書き込み
 *| param	: param_tbl - パラメータ情報テーブル		<IN>
 *|			  addr -  書き込み開始アドレス
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WritePParamTbl(const t_AppServ_ParamInfoTbl *param_tbl, ulong addr)
{
	ulong ofs, section, ret;
	t_AppServ_SectInfo	w;
	
	memset(&FLASH_WRITE_BUFFER[0] ,0xFF ,sizeof(FLASH_WRITE_BUFFER));
	ofs = FLT_HEADER_SIZE;
	/* セクション数 */
	memcpy(&FLASH_WRITE_BUFFER[ofs],(uchar*)&param_tbl->psection_num,sizeof(param_tbl->psection_num));
	ofs += sizeof(param_tbl->psection_num);
	/* 個別パラメータテーブル情報 */
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
 *| summary	: 個別パラメータのバックアップ領域への書き込み
 *| param	: param_tbl - パラメータ情報テーブル		<IN>
 *|			  addr -  書き込み開始アドレス
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WritePParamData(const t_AppServ_ParamInfoTbl *param_tbl, ulong addr)
{
	ushort	section;
	ulong	copysize, ofs, ret;

	/* 個別パラメータ */
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
 *| summary	: 共通パラメータのデータサイズ取得
 *| param	: param_tbl - パラメータ情報テーブル		<IN>
 *| return	: 共通パラメータデータサイズ
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
 *| summary	: バックアップデータをフラッシュに書き込み
 *| param	: area_tbl - バックアップデータ情報テーブル			<IN>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteAreaData(const t_AppServ_AreaInfoTbl *area_tbl)
{
	ulong ret, area, dst;
	
	/* データ書き込み */
	for (area = 0; area < area_tbl->area_num; area++) {
		if (area == OPE_DTNUM_LOCKINFO) {
			continue;
		}
		dst = area_tbl->from_area[area].from_address;
		if(area_tbl->from_area[area].from_address == 0L){
			continue;// アドレスが無い場合は格納対象外
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
 *| summary	: 設定パラメータリストア
 *| param	: param_tbl - 最新のパラメータ情報テーブル		<IN>
 *|			  param_tbl_wk - ワーク用パラメータ情報テーブル	<WORK>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_RestoreParam(const t_AppServ_ParamInfoTbl *param_tbl, t_AppServ_ParamInfoTbl *param_tbl_wk)
{
	ushort	section, section_num, item_num;
	
/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM2);
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	
/* フラッシュからテーブル読み込み */
	flt_ReadParamTbl(param_tbl_wk);
	
/* 共通パラメータ */
	/* セクション数の決定(少ないほうに合わせる) */
	section_num = param_tbl_wk->csection_num;
	if (param_tbl->csection_num < param_tbl_wk->csection_num) {
		section_num = param_tbl->csection_num;
	}
	for (section = 0; section < section_num; section++) {
		/* 項目数の決定(少ないほうに合わせる) */
		item_num = param_tbl_wk->csection[section].item_num;
		if (param_tbl->csection[section].item_num < param_tbl_wk->csection[section].item_num) {
			item_num = param_tbl->csection[section].item_num;
		}
		/* RAMへのデータ転送 */
		Flash2Read(param_tbl->csection[section].address,
				(ulong)param_tbl_wk->csection[section].address + FLT_PARAM_SECTOR,
				item_num * FLT_PARAM_DATA_SIZE);
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_RestoreLockParam()
 *[]----------------------------------------------------------------------[]
 *| summary	: 車室パラメータリストア
 *| param	: none
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_RestoreLockParam(void)
{
	ushort	lockmax, datasize, copysize, i;
	
/* データ有無のチェック */
	if (!flt_CheckSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	
/* フラッシュ上のデータ管理情報を読み込み */
	Flash2Read((uchar*)&lockmax, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_LOCKMAX, sizeof(lockmax));
	Flash2Read((uchar*)&datasize, FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_DATASIZE, sizeof(datasize));
/* 車室数・車室毎データサイズの決定(少ないほうに合わせる) */
	if (lockmax > LOCK_MAX) {
		lockmax = LOCK_MAX;
	}
	copysize = datasize;
	if (copysize > sizeof(t_LockInfo)) {
		copysize = sizeof(t_LockInfo);
	}
/* リストア実行 */
	memset(LockInfo, 0, sizeof(LockInfo));
	for (i = 0; i < lockmax; i++) {
		WACDOG;												// 装置ループの際はｳｫｯﾁﾄﾞｯｸﾘｾｯﾄ実行
		Flash2Read((uchar*)&LockInfo[i], 
				(ulong)(FLT_LOCKPARAM_SECTOR+FLT_LKPRMOFS_PARAM+(datasize*i)), (ulong)copysize);
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_GetLogRecordNum()
 *[]----------------------------------------------------------------------[]
 *| summary	: フラッシュ上のLOGレコード件数取得
 *| param	: logmgr - LOG管理データ					<IN>
 *| return	: LOGレコード件数
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
 *| summary	: LOGレコード取得
 *| param	: logmgr - LOG制御データ				<IN>
 *|			  handle - LOG検索ハンドル				<IN>
 *|			  buf - LOG取得バッファ					<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadLogRecord(const t_FltLogMgr *logmgr, const t_FltLogHandle *handle, char *buf)
{
	ulong src_addr;
	
	if (logmgr->sectmax <= handle->sectno) {
		return _FLT_MkRtnVal2(FLT_NODATA);	/* 指定位置にレコード無し */
	}
	if (handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		return _FLT_MkRtnVal2(FLT_NODATA);	/* 指定位置にレコード無し */
	}
	
	/* データ読み出し */
	src_addr = logmgr->sectinfo[handle->sectno].address 
				+ FLT_LOGOFS_RECORD
				+ (handle->recordno * logmgr->record_size);
	Flash2Read((uchar*)buf, src_addr, (ulong)logmgr->record_size);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}


/*[]----------------------------------------------------------------------[]
 *|	name	: flt_NextLog()
 *[]----------------------------------------------------------------------[]
 *| summary	: LOG検索ハンドル更新
 *| param	: logmgr - LOG制御データ					<IN>
 *|			  handle - LOG検索ハンドル					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_NextLog(const t_FltLogMgr *logmgr, t_FltLogHandle *handle)
{
	if (handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		return _FLT_MkRtnVal2(FLT_NODATA);	/* 現在位置にレコード無し */
	}
	/* 次レコードへ移動 */
	if (++handle->recordno >= logmgr->sectinfo[handle->sectno].record_num) {
		/* 今のセクタのレコードが終わりであれば次セクタへ移動 */
		handle->sectno = _offset(handle->sectno, 1, logmgr->sectmax);
		if (handle->sectno == _offset(logmgr->secttop, logmgr->sectnum, logmgr->sectmax)) {
			return _FLT_MkRtnVal2(FLT_NODATA);	/* 次レコードなし */
		}
		handle->recordno = 0;
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadLogo()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントロゴ取得
 *| param	: buf 							<OUT>
 *| 		  RaedType : 0=全部読み出し、1=部分読出し
 *| 		  TopOffset,ReadSize : 部分読出しの場合のみ有効。
 *| 		  					   読出し開始オフセットとサイズ。
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadLogo(char *buf, char RaedType, ushort TopOffset, ushort ReadSize)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_LOGO, FLT_HEADER_SIZE+FLT_PRINTLOGO_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	if( 1 == RaedType ){	// 部分読出し (Y)
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
 *| summary	: プリントヘッダ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadHeader(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_HEADER, FLT_HEADER_SIZE+FLT_PRINTHEADER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_HEADER+FLT_HEADER_SIZE, FLT_PRINTHEADER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadFooter()
 *[]----------------------------------------------------------------------[]
 *| summary	: プリントフッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadFooter(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FOOTER, FLT_HEADER_SIZE+FLT_PRINTFOOTER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FOOTER+FLT_HEADER_SIZE, FLT_PRINTFOOTER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadAccept1Footer()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受付券フッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadAcceptFooter(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_ACCEPTFOOTER, FLT_HEADER_SIZE+FLT_ACCEPTFOOTER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_ACCEPTFOOTER+FLT_HEADER_SIZE, FLT_ACCEPTFOOTER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Restore()
 *[]----------------------------------------------------------------------[]
 *| summary	: リストア実行
 *| param	: param_tbl - 最新のパラメータ情報テーブル				<IN>
 *|			  param_tbl_wk - ワーク用パラメータ情報テーブル			<WORK>
 *| 		  area_tbl - 最新のバックアップデータ情報テーブル		<IN>
 *|			  area_tbl_wk - ワーク用バックアップデータ情報テーブル	<WORK>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_Restore(const t_AppServ_ParamInfoTbl *param_tbl, 
					t_AppServ_ParamInfoTbl *param_tbl_wk,
					const t_AppServ_AreaInfoTbl *area_tbl, 
					t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong	 map_version, verup_flag, ret;
	t_FltReq dummy;
	
/* リストア対象データのサムチェック */
	/* リストア可能なデータがあるかチェック */
	if (!flt_GetVersion(&map_version, &verup_flag)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
/* 各データのリストア */
	/* パラメータ */
	flt_RestoreParam(param_tbl, param_tbl_wk);
	/* バックアップデータ */
	flt_Restore_BackupData(area_tbl, area_tbl_wk, param_tbl);
	if (map_version >= FLASH2_MAP_VERSION_0100) {	/* バージョンデータ無し時は車室パラメータ領域が存在しない(ver.0) */
	/* 車室パラメータ */
		flt_RestoreLockParam();
	}
	if (verup_flag != 0 || map_version != FLASH2_MAP_VERSION) {
	/* バージョン相違検出のため新MAPでバックアップ */
		ret = flt_WriteParam1(param_tbl, param_tbl_wk, FLT_Ctrl_Backup.workbuf, &dummy, FLT_CMP_ALL);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return ret;
		}
		/* 車室パラメータ */
		ret = flt_WriteLockParam1(FLT_Ctrl_Backup.workbuf, &dummy);
		if (_FLT_RtnKind(ret) != FLT_NORMAL) {
			return ret;
		}
		/* バックアップデータ */
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
 *| summary	: メモリマップバージョンの取得
 *| param	: mapver - 取得したMAPバージョンデータ					<OUT>
 *| 		  verup_flag - バージョンアップフラグ					<OUT>
 *| return	: FALSE - リストア用データなし
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_GetVersion(ulong *mapver, ulong *ver_flg)
{
	ulong vers;
	
	// バックアップテーブル情報のサムチェック
	if (!flt_CheckSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE)) {
		return FALSE;
	}
	/* MAPバージョン取得 */
	Flash2Read((uchar*)&vers, 
// 下行の「2」はt_AppServ_AreaInfoTbl->area_num(ushort)
				FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE+2+sizeof(t_AppServ_AreaInfo)*FLT_DTNUM_MAPVER, 
				sizeof(vers));
	/* 共通パラメータ領域SUMチェック */
	if (!flt_CheckSum(FLT_PARAM_SECTOR, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX)) {
		return FALSE;
	}
	if (vers >= FLASH2_MAP_VERSION_0100) {	/* バージョンデータ無し時は車室パラメータ領域が存在しない(ver.0) */
	/* 車室パラメータ領域SUMチェック */
		if (!flt_CheckSum(FLT_LOCKPARAM_SECTOR, FLT_LOCKPARAM_SECT_SIZE*FLT_LOCKPARAM_SECTORMAX)) {
			return FALSE;
		}
	}
	
	*mapver = vers;
	/* バージョンアップフラグ取得 */
	Flash2Read((uchar*)ver_flg, 
// 下行の「2」はt_AppServ_AreaInfoTbl->area_num(ushort)
				FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE+2+sizeof(t_AppServ_AreaInfo)*FLT_DTNUM_VERUPFLG, 
				sizeof(*ver_flg));
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_Restore_BackupData()
 *[]----------------------------------------------------------------------[]
 *| summary	: バックアップデータのリストア
 *| param	: area_tbl - 現状プログラムのバックアップデータ情報テーブル		<IN>
 *|			  area_tbl_wk - フラッシュ上のバックアップデータ情報テーブル	<WORK>
 *| return	: 0x00XXXXXX - 正常終了
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

/* フラッシュ上のバックアップデータ情報テーブル読み込み */
	flt_ReadAreaTbl(area_tbl_wk);
	
	// バックアップデータのリストア
	AppServ_Restore_BackupData(area_tbl, area_tbl_wk);
	if( AppServ_PParam_Copy == ON ){						// FLASH ⇒ RAM 個別ﾊﾟﾗﾒｰﾀｺﾋﾟｰ有無？
/* 個別パラメータテーブルのリストア */
		src_addr = FLT_BACKUP_SECTOR26 + FLT_HEADER_SIZE;
		src_addr2 = FLT_BACKUP_SECTOR27 + FLT_HEADER_SIZE;
// MH810103(s) 営休業状態がリストアされない不具合修正
//// MH810103 GG119202(S) 現在台数がリストアされない
//		if (bk_log_ver == 1) {
//			// LOG_VERSION=1(GG1162)からリストアする場合は
//			// 個別パラメータのバックアップセクタがずれているので
//			// LOG_VERSION=1のときのバックアップセクタからリストアする
//			src_addr = (FLT_BACKUP_SECTOR26 + (FLT_BACKUP_SECT_SIZE*4)) + FLT_HEADER_SIZE;
//			src_addr2 = (FLT_BACKUP_SECTOR27 + (FLT_BACKUP_SECT_SIZE*4)) + FLT_HEADER_SIZE;
//		}
// MH810103 GG119202(E) 現在台数がリストアされない
// MH810103(e) 営休業状態がリストアされない不具合修正
		Flash2Read((uchar*)&section_num, src_addr, sizeof(section_num));
		if (section_num != 0xFFFF) {
			src_addr += sizeof(section_num);
			for (section = 0; section < section_num; section++) {
				Flash2Read((uchar*)&sectinfo, src_addr, sizeof(sectinfo));
				/* データサイズの決定(少ないほうに合わせる) */
				datasize = param_tbl->psection[section].item_num;
				if (datasize > sectinfo.item_num) {
					datasize = sectinfo.item_num;
				}
				datasize *= FLT_PARAM_DATA_SIZE;
				/* RAMへのデータ転送 */
				Flash2Read_WpOff(param_tbl->psection[section].address, 
					src_addr2,
					datasize);
				src_addr += sizeof(sectinfo);
				src_addr2 += datasize;
			}
			prm_oiban_chk();
		}
		else {
			/* 個別パラメータデータ無し(ver0.0のメモリマップ) */
		}
	}
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParamSector()
 *[]----------------------------------------------------------------------[]
 *| summary	: パラメータセクタ書き込み
 *| param	: address - 書込み先アドレス
 *|			  data - 書き込むデータ
 *|			  size - 書き込むサイズ
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParamSector(ulong address, uchar *data, ulong size)
{
	ulong ret;
	ulong n;

	address &= 0xFFFFF000UL;			// アドレスをセクタ先頭に補正
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
 *| summary	: セクタ書き込み
 *| param	: address - 書込み先アドレス
 *|			  data - 書き込むデータ
 *|			  size - 書き込むサイズ
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteSector(ulong address, uchar *data, ulong size)
{
	ulong ret;

/* セクタイレース */
	ret = Flash2EraseSector(address);
	if (ret != FROM2_NORMAL) {
		return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
	}
/* データ書き込み */
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
 *| summary	: プリントデータ(ロゴ、ヘッダ、フッタ)書き込み
 *| param	: data - 書き込むデータ
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WritePrintDataSector(uchar *data)
{
	ulong ret;
	
/* セクタイレース */
	ret = flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
	if (ret != FLT_NORMAL) {
		return _FLT_MkRtnVal(FLT_ERASE_ERR, ret);
	}

	// まとめて書き込みする
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
 *| summary	: サム付きのデータをフラッシュ書き込み
 *| param	: address - 書込み先アドレス
 *|			  data - 書き込むデータ
 *|			  size - 書き込むサイズ
 *| return	: 0x00XXXXXX - 正常終了
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
 *| summary	: サム作成
 *| param	: buf - 処理対象領域
 *|			  size - 領域サイズ
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
 *| summary	: サム計算(sumcal()のサイズ引数がshortなのでulong版を自前で用意)
 *| param	: data - サム算出対象領域			<IN>
 *|			  size - 書き込むサイズ
 *| return	: サム
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
 *| summary	: パラメータ情報テーブルのフォーマット変換(⇒フラッシュフォーマット[=セクション数分のみデータあり]形式)
 *| param	: buf - コピー先バッファ
 *|			  param_tbl - パラメータ情報テーブル			<IN>
 *| return	: 変換後のパラメータ情報テーブルサイズ
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteParamTbl(uchar *buf, const t_AppServ_ParamInfoTbl *param_tbl)
{
	ushort	section;
	ulong	size;
	ulong	ofs;	/* セクタ先頭からのオフセットアドレス */
	t_AppServ_SectInfo	w;
	
	size	= 0;
	ofs		= flt_ParamTblSize(param_tbl) + FLT_HEADER_SIZE;
	
	/* 共通パラメータセクション数 */
	memcpy(buf, &param_tbl->csection_num, sizeof(param_tbl->csection_num));
	size += sizeof(param_tbl->csection_num);
	/* 個別パラメータセクション数 */
	memset(&buf[size], 0, sizeof(param_tbl->psection_num));
	size += sizeof(param_tbl->psection_num);
	/* 共通パラメータセクション情報 */
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
 *| summary	: フラッシュからパラメータ情報テーブルを読み込む
 *| param	: param_tbl_wk - コピー先バッファ				<OUT>
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
	
	/* 共通パラメータセクション数 */
	Flash2Read((uchar*)&param_tbl_wk->csection_num, src_addr, sizeof(param_tbl_wk->csection_num));
	src_addr += sizeof(param_tbl_wk->csection_num);
	/* 個別パラメータセクション数 */
	Flash2Read((uchar*)&param_tbl_wk->psection_num, src_addr, sizeof(param_tbl_wk->psection_num));
	src_addr += sizeof(param_tbl_wk->psection_num);
	/* 共通パラメータセクション情報 */
	for (section = 0; section < param_tbl_wk->csection_num; section++) {
		Flash2Read((uchar*)&param_tbl_wk->csection[section], src_addr, sizeof(param_tbl_wk->csection[0]));
		src_addr += sizeof(param_tbl_wk->csection[0]);
	}
	/* 個別パラメータセクション情報 */
	for (section = 0; section < param_tbl_wk->psection_num; section++) {
		Flash2Read((uchar*)&param_tbl_wk->psection[section], src_addr, sizeof(param_tbl_wk->psection[0]));
		src_addr += sizeof(param_tbl_wk->psection[0]);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ParamCopy()
 *[]----------------------------------------------------------------------[]
 *| summary	: param_tblの情報を元に設定パラメータデータをdstが示すアドレスにコピーする
 *| param	: dst - 設定パラメータデータのコピー先
 *|			  param_tbl - パラメータ情報テーブル
 *| return	: TRUE - コピー成功
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_ParamCopy(uchar *dst, const t_AppServ_ParamInfoTbl *param_tbl)
{
	ushort	section;
	ulong	copysize;
	
	/* 共通パラメータ */
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
 *| summary	: パラメータ情報テーブルのフラッシュ上でのサイズを取得する
 *| param	: param_tbl - パラメータ情報テーブル
 *| return	: パラメータ情報テーブルのフラッシュ上でのサイズ
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
 *| summary	: バックアップデータ情報テーブルをフラッシュに書き込み
 *|				(フラッシュフォーマット[=エリア数分のみデータあり]形式で書き込み)
 *| param	: area_tbl - バックアップデータ情報テーブル			<IN>
 *| return	: フラッシュ上のバックアップデータ情報テーブルサイズ
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
// 各エリアごとの情報の書き込み
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
// 対象データFROM保存領域情報の書き込み
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
 *| summary	: フラッシュからバックアップデータ情報テーブルを読み込む
 *| param	: param_tbl_wk - コピー先バッファ				<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_ReadAreaTbl(t_AppServ_AreaInfoTbl *area_tbl_wk)
{
	ulong	src_addr;
	ushort	area;
	
	src_addr = FLT_BACKUP_SECTOR0+FLT_HEADER_SIZE;
	
	/* エリア数 */
	Flash2Read((uchar*)&area_tbl_wk->area_num, src_addr, sizeof(area_tbl_wk->area_num));
	src_addr += sizeof(area_tbl_wk->area_num);
	/* 各エリアの情報 */
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
 *| summary	: 連続した複数セクタイレース
 *| param	: top - セクタ先頭
 *|			  sector_size - セクタサイズ
 *|			  sector_num - セクタ数
 *| return	: 0x00XXXXXX - 正常終了
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
 *| summary	: kindで指定された領域をロックする
 *|			  wait_flag!=0であれば、必ずロックしてからリターンする
 *| param	: req_tbl - 要求管理テーブル				<IN/OUT>
 *|			  kind - 要求先領域(FLT_REQTBL_XXXX)
 *|			  req - 要求種別(FLT_REQ_WRITE/FLT_REQ_READ)
 *| return	: TRUE - ロックできた
 *|-----------------------------------------------------------------------[]
 *| remark	: 本関数ではロック後にtskchgをコールしないので、関数リターン時点では、
 *|			  フラッシュタスクはその動作を始めていない
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

	// ログデータの場合は一定時間リトライする
	//	if(kind >= (uchar)FLT_REQTBL_LOG && kind < (uchar)FLT_REQTBL_PARAM1) {	// 対象はログデータ(ワーニングが出る)
	if(kind < FLT_REQTBL_PARAM1) {								// 対象はログデータ
		ret = TRUE;												// 一旦TRUEに変更
		startTime = LifeTimGet();
		while(!flt_CheckBusy(req_tbl, kind, req, flag)) {		// ロックリトライ
			taskchg( IDLETSKNO );								// task switch (a little bit wait)
			if( 9000 <= LifePastTimGet(startTime) ){			// 90sec(音声再生中のセマフォ待ち時間と同じ)経ってもロックできない場合はあきらめる
				errdata = kind;
				Err_work.Errinf = 2;
				Err_work.ErrBinDat = errdata;
				err_chk2(ERRMDL_MAIN, ERR_MAIN_LOG_WRITE_LOCK, 2, 2, 1, &errdata);	// ログ書込み時のロック失敗(発生/解除) エラー情報=要求先領域
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
 *| summary	: kindで指定された領域のロックを解除する
 *| param	: kind - 要求先領域(FLT_REQTBL_XXXX)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	flt_UnlockFlash(t_FltReq *req_tbl, uchar kind)
{
	if (kind == FLT_REQTBL_PARAM1 || kind == FLT_REQTBL_LOCKPARAM1) {
		if (req_tbl[kind].req == FLT_REQ_WRITE) {
			return;		/* 書き込み中に再度要求が行われたので再書込みに行く */
		}
	}
	req_tbl[kind].req = FLT_REQ_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_CheckBusy()
 *[]----------------------------------------------------------------------[]
 *| summary	: kindで指定された領域がアクセス可能かどうかをチェックする
 *| param	: req_tbl - 要求管理テーブル				<IN>
 *|			  kind - 要求先領域(FLT_REQTBL_XXXX)
 *|			  req - 要求種別(FLT_REQ_WRITE/FLT_REQ_READ)
 *| return	: TRUE - アクセス可能
 *[]----------------------------------------------------------------------[]*/
BOOL	flt_CheckBusy(const t_FltReq *req_tbl, uchar kind, uchar req, uchar flag)
{
	int		i;
	BOOL	ret = FALSE;

	if( kind < FLT_REQTBL_LOG+LOG_STRAGEN ){
		/* 検索ハンドルオープン～ハンドルクローズまでLOG書き換えが起こらないことを保証 */
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
	/* FLASHタスクでの実行を要求のケース */
			if (kind == FLT_REQTBL_PARAM1 || kind == FLT_REQTBL_LOCKPARAM1) {
				ret = TRUE;		/* 必ず受け付ける要求 */
			}
			else {
				if (req_tbl[kind].req == FLT_REQ_IDLE) {
					if (kind == FLT_REQTBL_BACKUP) {
						/* 検索ハンドルオープン～ハンドルクローズまでLOG書き換えが起こらないことを保証 */
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
	/* API呼び出し側コンテキストでのタスク排他実行を要求のケース */
			switch (kind) {
			case FLT_REQTBL_BACKUP:
			case FLT_REQTBL_RESTORE:
			case FLT_REQTBL_BRLOGSZ:
				// ログ以外がIDLEであるかをチェックする
				i = FLT_REQTBL_PARAM1;
				break;
			default:
				// 全ての要求がIDLEであるかをチェックする
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
 *| summary	: RAM上の指定されたLOGエリアの消去を行う。
 *| param	: kind  eLog種別
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	FLT_LogErase2( unsigned char Kind )
{
	ushort	i;

	memset( &LOG_DAT[Kind], 0, sizeof(struct log_record) );
	// ニアフル関連情報を再設定する
	for(i = 0; i < eLOG_TARGET_MAX; ++i) {
		LOG_DAT[Kind].nearFull[i].NearFullMaximum = Ope_Log_GetNearFullCount(Kind);
		LOG_DAT[Kind].nearFull[i].NearFullMinimum = 0;
		LOG_DAT[Kind].nearFull[i].NearFullStatus = RAU_NEARFULL_CANCEL;
		LOG_DAT[Kind].nearFull[i].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
	}
	// ログを消去したのでRAUタスクにニアフル（解除）チェックを要求する
	if(RAUdata_CanSendTableData(Kind)) {
		RAU_RequestCheckNearfull();
	}
	
	/* FlashROM内 LOG制御(管理)データ(RAM)のクリア */
	if(Kind < LOG_STRAGEN){// FROMに割り当てられているログのみLOG制御(管理)データをクリアする
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
 *| summary	: FlashROM内にバックアップされている FLAPDT データのリストア
 *| param	: none
 *| return	: 0x00XXXXXX - 正常終了
 *|           0x06XXXXXX - FLT_NODATA (FlashROM内にデータなし)
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_Restore_FLAPDT( void )
{
	ushort	area;
	ulong	datasize;
	const t_AppServ_AreaInfoTbl *area_tbl = &FLT_Ctrl_NotBackup.area_tbl;
	t_AppServ_AreaInfoTbl *area_tbl_wk    = &FLT_Ctrl_NotBackup.area_tbl_wk;


	_flt_DisableTaskChg();		// タスク切替なしで Flash access実行

	// テーブル情報のサムチェック
	if (!flt_CheckSum(FLT_BACKUP_SECTOR0, FLT_BACKUP_SECT_SIZE)) {
		_flt_EnableTaskChg();
		return _FLT_MkRtnVal2(FLT_NODATA);
	}

	/* フラッシュ上のバックアップデータ情報テーブル読み込み */
	flt_ReadAreaTbl(area_tbl_wk);

	/* バックアップデータのリストア */
	area = OPE_DTNUM_FLAPDT;

	/* データサイズの決定(少ないほうに合わせる) */
	datasize = area_tbl->area[area].size;
	if (datasize > area_tbl_wk->area[area].size) {
		datasize = area_tbl_wk->area[area].size;
	}

	/* RAMへのデータ転送 */
	Flash2Read(area_tbl->area[area].address, 
			area_tbl_wk->from_area[area].from_address,
			datasize);

	_flt_EnableTaskChg();
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: Flt_Comp_ComParaSize_FRom_to_SRam()
 *[]----------------------------------------------------------------------[]
 *| summary	: FlashROM内とSRAM内の共通ﾊﾟﾗﾒｰﾀｴﾘｱのｾｸｼｮﾝ数とｱｲﾃﾑ数を比較する。
 *|           ※この関数は FLT_init() より後でCallすること。
 *| param	: none
 *| return	: 0 = FlashROM内に共通ﾊﾟﾗﾒｰﾀｴﾘｱなし
 *|           1 = FlashROMとSRAMでは同じｾｸｼｮﾝ数とｱｲﾃﾑ数
 *|           2 = FlashROMとSRAMではｾｸｼｮﾝ数 又は ｱｲﾃﾑ数が異なる
 *[]----------------------------------------------------------------------[]*/
uchar	FLT_Comp_ComParaSize_FRom_to_SRam( void )
{
	t_AppServ_ParamInfoTbl *pSRAM_param_tbl;			// SRAMのﾊﾟﾗﾒｰﾀ情報ﾃｰﾌﾞﾙ
	t_AppServ_ParamInfoTbl *pFLASH_param_tbl;			// Flash内のﾊﾟﾗﾒｰﾀ情報ﾃｰﾌﾞﾙ
	ushort	section, section_max;
	uchar	ret = 1;									// ｾｸｼｮﾝ数，ｱｲﾃﾑ数共に同じ

	_flt_DisableTaskChg();								// ﾀｽｸ切替無し

	/* Flashからテーブル読み込み */
	pFLASH_param_tbl = &FLT_Ctrl_NotBackup.param_tbl_wk;
	flt_ReadParamTbl( pFLASH_param_tbl );

	pSRAM_param_tbl = &FLT_Ctrl_Backup.param_tbl;

	/* ｾｸｼｮﾝ数比較 */
	if (pSRAM_param_tbl->csection_num != pFLASH_param_tbl->csection_num) {
		ret = 2;										// ｾｸｼｮﾝ数が違う
		goto FLT_Comp_ComParaSize_FRom_to_SRam_10;
	}

	/* ｾｸｼｮﾝ毎 ｱｲﾃﾑ数比較 */
	section_max = pSRAM_param_tbl->csection_num;		// ｾｸｼｮﾝ数get

	for (section = 0; section < section_max; section++) {
		if (pSRAM_param_tbl->csection[section].item_num != pFLASH_param_tbl->csection[section].item_num) {
			ret = 2;									// ｾｸｼｮﾝ数が違う
			goto FLT_Comp_ComParaSize_FRom_to_SRam_10;
		}
	}

FLT_Comp_ComParaSize_FRom_to_SRam_10:
	_flt_EnableTaskChg();								// ﾀｽｸ切替なし解除
	return	ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット署名欄取得（印刷用）
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadSyomei(char *buf)
{
	ulong ret;
	
	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadSyomei(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット署名欄取得（印刷用）
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadSyomei(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_SYOMEI, FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_SYOMEI+FLT_HEADER_SIZE, FLT_PRINTSYOMEI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadKamei()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット加盟店取得（印刷用）
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadKamei(char *buf)
{
	ulong ret;
	
	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadKamei(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadKamei()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット加盟店取得（印刷用）
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadKamei(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_KAMEI, FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_KAMEI+FLT_HEADER_SIZE, FLT_PRINTKAMEI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}


/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット署名欄書き込み（印字用）
 *| param	: image - 署名欄データ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ タスク初期化前か否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteSyomei(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_SYOMEI, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.syomei.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteSyomei(&FLT_Ctrl_Backup.params.syomei,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_SYOMEI]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_SYOMEI);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_SYOMEI);
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteSyomei()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット署名欄書き込み（印字用）
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteSyomei(const t_FltParam_Syomei *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);

	/* ロゴ、ヘッダ、フッタ、署名欄、加盟店領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */

	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_SYOMEI+FLT_HEADER_SIZE], params->image, FLT_PRINTSYOMEI_SIZE);

	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_SYOMEI], FLT_HEADER_SIZE+FLT_PRINTSYOMEI_SIZE);
	/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
	/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* ベリファイ */
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
 *| summary	: クレジット加盟店書き込み（印字用）
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteKamei(const t_FltParam_Kamei *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ、署名欄、加盟店領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */

	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_KAMEI+FLT_HEADER_SIZE], params->image, FLT_PRINTKAMEI_SIZE);

	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_KAMEI], FLT_HEADER_SIZE+FLT_PRINTKAMEI_SIZE);
	/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
	/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* ベリファイ */
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
 *| summary	: クレジット加盟店書き込み（印字用）
 *| param	: image - 加盟店データ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ タスク初期化前か否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteKamei(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_KAMEI, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.kamei.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteKamei(&FLT_Ctrl_Backup.params.kamei,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_KAMEI]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_KAMEI);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_KAMEI);
	}
	
	return ret;
}

// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: ＥＤｙＡＴコマンド取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadEdyAt(char *buf, uchar flags)
{
	ulong ret;
	
	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadEdyAt(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: ＥＤｙＡＴコマンド取得（本処理）
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadEdyAt(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EDYAT, FLT_HEADER_SIZE + FLT_EDYATCMD_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EDYAT+FLT_HEADER_SIZE, FLT_EDYATCMD_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: ＥＤｙＡＴコマンド書き込み
 *| param	: image - ＥＤｙＡＴコマンドデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ タスク初期化前か否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteEdyAt(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EDYAT, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.edyat.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteEdyAt(&FLT_Ctrl_Backup.params.edyat,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EDYAT]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EDYAT);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_EDYAT);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteEdyAt()
 *[]----------------------------------------------------------------------[]
 *| summary	: ＥＤｙＡＴコマンド書き込み（フラッシュタスクを使わない書込み）
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteEdyAt(const t_FltParam_EdyAt *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_EDYAT+FLT_HEADER_SIZE], params->image, FLT_EDYATCMD_SIZE);

	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_EDYAT], FLT_HEADER_SIZE+FLT_EDYATCMD_SIZE);

/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {

/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {

/* ベリファイ */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	return ret;
}
// MH321800(S) G.So ICクレジット対応 不要機能削除(Edy)
//#endif
// MH321800(E) G.So ICクレジット対応 不要機能削除(Edy)

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ユーザ定義情報読み出し
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadUserDef(char *buf)
{
	ulong ret;
	
	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadUserDef(buf);
	_flt_EnableTaskChg();
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ユーザ定義情報読み出し
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
static
ulong	flt_ReadUserDef(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_USERDEF, FLT_HEADER_SIZE+FLT_USERDEF_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_USERDEF+FLT_HEADER_SIZE, FLT_USERDEF_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ユーザ定義情報書き込み
 *| param	: image - 加盟店データ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ タスク初期化前か否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteUserDef(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_USERDEF, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.userdef.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteUserDef(&FLT_Ctrl_Backup.params.userdef,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_USERDEF]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_USERDEF);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_USERDEF);
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteUserDef()
 *[]----------------------------------------------------------------------[]
 *| summary	: ユーザ定義情報書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
static
ulong	flt_WriteUserDef(const t_FltParam_UserDef *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ、署名欄、加盟店領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */

	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_USERDEF+FLT_HEADER_SIZE], params->image, FLT_USERDEF_SIZE);

	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_USERDEF], FLT_HEADER_SIZE+FLT_USERDEF_SIZE);
	/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
	/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* ベリファイ */
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
 *| summary	: サム作成
 *| param	: buf - 処理対象領域
 *|			  size - 領域サイズ
 *| return	: none
 *| NOTE    : flt_SetSum()と同じだが、後でatchdog対策を追加できるよう１層設けておく
 *[]----------------------------------------------------------------------[]*/
void	FLT_SetSum(uchar *buf, ulong size)
{
	flt_SetSum(buf, size);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadEtc()
 *[]----------------------------------------------------------------------[]
 *| summary	: ETCデータ(=FLT_PRINTDATA_SECTOR)読み出し
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadEtc(char *buf)
{
	/* 本処理実行 */
	_flt_DisableTaskChg();
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR, _ETC_SIZE_);
	_flt_EnableTaskChg();
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteEtc()
 *[]----------------------------------------------------------------------[]
 *| summary	: ETCデータ(=FLT_PRINTDATA_SECTOR)書き込み
 *| param	: buf 							<IN>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteEtc(char *buf)
{
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_ETC, FLT_REQ_WRITE, FLT_NOT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.etc.image = buf;

	/* 書き込み完了は外で待つ */
	return _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_ETC);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteEtc()
 *[]----------------------------------------------------------------------[]
 *| summary	: ETCデータ(=FLT_PRINTDATA_SECTOR)書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
static
ulong	flt_WriteEtc(const t_FltParam_Etc *params, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* 書き込み実行 */
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
	ret = flt_WritePrintDataSector((uchar*)params->image);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
	/* ベリファイ */
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
 *| summary	: イメージデータ書き込み
 *| param	: buf - 書き込みデータ				<IN>
 *|			  len -	データ長					<IN>
 *|			  flash	-	書き込み先				<IN>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteImage(char *buf, ulong len, ulong flash, int erase)
{
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_IMAGE, FLT_REQ_WRITE, FLT_NOT_EXCLUSIVE)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.image.image = buf;
	FLT_Ctrl_Backup.params.image.len = len;
	FLT_Ctrl_Backup.params.image.flash = flash;
	FLT_Ctrl_Backup.params.image.erase = erase;

	/* 書き込み完了は外で待つ */
	return _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_IMAGE);
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteImage()
 *[]----------------------------------------------------------------------[]
 *| summary	: イメージデータ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
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
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
	if( DOWNLOADING() ){											// 遠隔ダウンロード時
		if( remotedl_exec_info_get() == FLASH_WRITE_BEFORE ){		// 前回情報が書込み前
			exec = FLASH_FIRST_WRITE_START;							// 初回書込みとする
		}else{
			exec = FLASH_WRITE_START;								// 部分書込みとする
		}
		remotedl_exec_info_set( exec );								// 処理ステータスを保持
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
 *| summary	: 直接書き込み
 *| param	: buf - 書き込みデータ				<IN>
 *|			  len -	データ長					<IN>
 *|			  flash	-	書き込み先				<IN>
 *| return	: 0x00XXXXXX - 正常終了
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
 *| summary	: 待機面パラメータ書き込み(引数にバイナリイメージ付き)
 *| param	: param_image - パラメータ保存イメージ		<IN>
 *|			  image_size - param_imageの有効データ長
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteParam3(const char *param_image, size_t image_size, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM3, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.param3.param_image	= param_image;
	FLT_Ctrl_Backup.params.param3.image_size	= flt_CParamDataSize(&FLT_Ctrl_Backup.param_tbl);
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteParam3(&FLT_Ctrl_Backup.params.param3, 
							&FLT_Ctrl_Backup.param_tbl,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_PARAM3]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_PARAM3);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_PARAM3);
	}
	
	return ret;
}
/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteParam3()
 *[]----------------------------------------------------------------------[]
 *| summary	: 待機面パラメータ書き込み(引数にバイナリイメージ付き)
 *| param	: params - APIユーザからの引数				<IN>
 *|			  param_tbl - パラメータ情報テーブル		<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
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

/* 書き込みイメージ作成 */
	memset(wbuf, 0xFF, (FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX));
	len = FLT_HEADER_SIZE;
	/* パラメータ情報テーブル */
	len += flt_WriteParamTbl(&wbuf[len], param_tbl);
	if (len + params->image_size > FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX) {
		return _FLT_MkRtnVal2(FLT_INVALID_SIZE);
	}
	/* パラメータデータ本体 */
	memcpy(&wbuf[len], params->param_image, params->image_size);
	/* サム */
	flt_SetSum(wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
	ret = flt_WriteParamSector(sect_addr, wbuf, FLT_PARAM_SECT_SIZE*FLT_PARAM_SECTORMAX);
	if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
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
 *| summary	: 待機面パラメータテーブル読出し
 *| param	: param_tbl - パラメータ情報テーブル		<IN/OUT>
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
	
/* 共通パラメータ */
	section_num = param_tbl_wk->csection_num;
	for (section = 0; section < section_num; section++) {
		param_tbl_wk->csection[section].address =
			(void*)((ulong)param_tbl_wk->csection[section].address + FTP_buff);
	}
	Flash2Read( (uchar*)FTP_buff, sect_addr, 65536 );
/* 個別パラメータ */
	// 省略
}

//[]----------------------------------------------------------------------[]
///	@brief			精算LOG取得(先頭のレコード)
//[]----------------------------------------------------------------------[]
///	@param[out]		handle	: 検索ハンドル
///	@param[out]		buf		: 最古の精算LOGデータ1件分のデータ
///	@return			0x00XXXXXX	: 正常終了
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindFirstEpayLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			精算LOGの登録時刻だけ取得(2件目以降のレコード)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	handle	: 検索ハンドル
///	@param[out]		buf		: 精算LOG2件目以降のデータ
///	@return			0x00XXXXXX	: 正常終了
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindNextEpayLog(t_FltLogHandle *handle, char *buf)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			精算LOGの登録時刻と電子マネー種別だけ取得(先頭のレコード)
//[]----------------------------------------------------------------------[]
///	@param[out]		handle		: 検索ハンドル
///	@param[out]		buf			: 最古の精算LOGデータ1件分のデータ
///	@param[in]		seek_cnt	: アクセス完了後のシーク動作
///	@param[in]		offset_num	: 最古から飛ばす情報件数
///	@return			0x00XXXXXX	: 正常終了
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindFirstEpayKnd(t_FltLogHandle *handle, char *buf, ushort seek_cnt, ushort offset_num)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			精算LOGの登録時刻と電子マネー種別だけ取得(2件目以降のレコード)
//[]----------------------------------------------------------------------[]
///	@param[in/out]	handle		: 検索ハンドル
///	@param[out]		buf			: 精算LOG2件目以降のデータ
///	@param[in]		seek_num	: アクセス完了後のシーク動作
///	@return			0x00XXXXXX	: 正常終了
//[]----------------------------------------------------------------------[]
///	@author			ise.y
///	@date			Create	: 2008/11/11<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
ulong	FLT_FindNextEpayKnd(t_FltLogHandle *handle, char *buf, ushort seek_cnt)
{ return 0L; }	// not serviced

//[]----------------------------------------------------------------------[]
///	@brief			FlashROM順次アクセス用シーク処理
//[]----------------------------------------------------------------------[]
///	@param[out]		handle	: 検索ハンドル
///	@param[out]		num		: 進める情報件数
///	@return			0x00XXXXXX	: 正常終了
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
 *| summary	: 預り証フッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadAzuFtr(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadAzuFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadAzuFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 預り証フッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadAzuFtr(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_AZUFTR, FLT_HEADER_SIZE+FLT_AZUFTR_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_AZUFTR+FLT_HEADER_SIZE, FLT_AZUFTR_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteAzuFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 預り証フッタ書き込み
 *| param	: image - プリントフッタデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteAzuFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_AZUFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.azuftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteAzuFtr(&FLT_Ctrl_Backup.params.azuftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_AZUFTR]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_AZUFTR);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_AZUFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteAzuFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 預り証フッタ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteAzuFtr(const t_FltParam_AzuFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_AZUFTR+FLT_HEADER_SIZE], params->image, FLT_AZUFTR_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_AZUFTR], FLT_HEADER_SIZE+FLT_AZUFTR_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}

// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット過払いフッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadCreKbrFtr(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadCreKbrFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット過払いフッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadCreKbrFtr(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_CREKBRFTR, FLT_HEADER_SIZE+FLT_CREKABARAI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_CREKBRFTR+FLT_HEADER_SIZE, FLT_EPAYKABARAI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット過払いフッタ書き込み
 *| param	: image - プリントフッタデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteCreKbrFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_CREKBRFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.crekbrftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteCreKbrFtr(&FLT_Ctrl_Backup.params.crekbrftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_CREKBRFTR]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_CREKBRFTR);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_CREKBRFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteCreKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: クレジット過払いフッタ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteCreKbrFtr(const t_FltParam_CreKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_CREKBRFTR+FLT_HEADER_SIZE], params->image, FLT_CREKABARAI_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_CREKBRFTR], FLT_HEADER_SIZE+FLT_CREKABARAI_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
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
 *| summary	: 電子マネー過払いフッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadEpayKbrFtr(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadEpayKbrFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadEpayKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 電子マネー過払いフッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadEpayKbrFtr(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EPAYKBRFTR, FLT_HEADER_SIZE+FLT_EPAYKABARAI_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EPAYKBRFTR+FLT_HEADER_SIZE, FLT_EPAYKABARAI_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteEpayKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 電子マネー過払いフッタ書き込み
 *| param	: image - プリントフッタデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteEpayKbrFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EPAYKBRFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.epaykbrftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteEpayKbrFtr(&FLT_Ctrl_Backup.params.epaykbrftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EPAYKBRFTR]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EPAYKBRFTR);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_EPAYKBRFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteEpayKbrFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 電子マネー過払いフッタ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteEpayKbrFtr(const t_FltParam_EpayKbrFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_EPAYKBRFTR+FLT_HEADER_SIZE], params->image, FLT_EPAYKABARAI_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_EPAYKBRFTR], FLT_HEADER_SIZE+FLT_EPAYKABARAI_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する

// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_ReadFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 後日支払額フッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadFutureFtr(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadFutureFtr(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 後日支払額フッタ取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadFutureFtr(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FUTUREFTR, FLT_HEADER_SIZE+FLT_FUTURE_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_FUTUREFTR+FLT_HEADER_SIZE, FLT_FUTURE_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 後日支払額フッタ書き込み
 *| param	: image - プリントフッタデータ		<IN>
 *|			  flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteFutureFtr(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FUTUREFTR, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.futureftr.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteFutureFtr(&FLT_Ctrl_Backup.params.futureftr,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_FUTUREFTR]);
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_FUTUREFTR);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_FUTUREFTR);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteFutureFtr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 後日支払額フッタ書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteFutureFtr(const t_FltParam_FutureFtr *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_FUTUREFTR+FLT_HEADER_SIZE], params->image, FLT_FUTURE_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_FUTUREFTR], FLT_HEADER_SIZE+FLT_FUTURE_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				flt_EraseSectors( FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE, FLT_PRINTDATA_SECTORMAX );
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}
	
	return ret;
}
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター

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
		return 0;	/* ロックできなかった */
	}
	/* パラメータを保存 */
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
 *| summary	: ログサイズ情報取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_ReadBRLOGSZ( ushort *buf )
{
	ulong ret;

	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadBRLOGSZ(buf);
	_flt_EnableTaskChg();

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_ReadBRLOGSZ()
 *[]----------------------------------------------------------------------[]
 *| summary	: ログサイズ情報取得
 *| param	: buf 							<OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_ReadBRLOGSZ( ushort *buf )
{
	/* データ有無のチェック */
	if( !flt_CheckSum( FLT_BRLOGSZ_SECTOR, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE ) ){
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read( (uchar*)buf, FLT_BRLOGSZ_SECTOR+FLT_HEADER_SIZE, FLT_BR_LOG_SIZE);

	return _FLT_MkRtnVal2(FLT_NORMAL);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: FLT_WriteBRLOGSZ()
 *[]----------------------------------------------------------------------[]
 *| summary	: ログサイズ情報書き込み
 *| param	: flags - API動作設定
 *|				FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ 他タスクを排他するか否か
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	FLT_WriteBRLOGSZ( char *buf, uchar flags )
{
	ulong	ret;

	/* 処理対象領域のロック */
	if( !flt_LockFlash( FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BRLOGSZ, FLT_REQ_WRITE, flags ) ){
		return _FLT_MkRtnVal2( FLT_BUSY );	/* ロックできなかった */
	}
	/* パラメータを保存 */
	memcpy( &FLT_Ctrl_Backup.params.brlogsz[0].log_size, buf, FLT_BR_LOG_SIZE );

	if( flags & FLT_EXCLUSIVE ){
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();
		ret = flt_WriteBRLOGSZ( &FLT_Ctrl_Backup.params.brlogsz[0],
								 FLT_Ctrl_Backup.workbuf,
								&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_BRLOGSZ] );
		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash( FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_BRLOGSZ );
	}else{
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal( FLT_NORMAL, FLT_REQTBL_BRLOGSZ );
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: flt_WriteBRLOGSZ()
 *[]----------------------------------------------------------------------[]
 *| summary	:ログサイズ情報書き込み
 *| param	: params - APIユーザからの引数				<IN>
 *|			  wbuf - 停電保証対応のワーク領域			<WORK>
 *|			  req_tbl - 要求受付情報					<IN/OUT>
 *| return	: 0x00XXXXXX - 正常終了
 *[]----------------------------------------------------------------------[]*/
ulong	flt_WriteBRLOGSZ(const t_FltParam_BRLOGSZ *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);

/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy( &wbuf[FLT_HEADER_SIZE], &params->log_size, FLT_BR_LOG_SIZE );
	/* SUMを作成 */
	flt_SetSum( wbuf, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE );
/* 書き込み実行 */
	req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
	ret = flt_WriteSector( FLT_BRLOGSZ_SECTOR, wbuf, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE);
	if( _FLT_RtnKind(ret) == FLT_NORMAL ){
/* ベリファイ */
		if ( Flash2Cmp(FLT_BRLOGSZ_SECTOR, wbuf, FLT_HEADER_SIZE+FLT_BR_LOG_SIZE) != 0 ) {
			Flash2EraseSector( FLT_BRLOGSZ_SECTOR );
			ret = _FLT_MkRtnVal2( FLT_VERIFY_ERR );
		}
	}

	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM上の音声データ運用面書き換え処理
//[]----------------------------------------------------------------------[]
///	@param[in]		運用面情報 OPERATE_AREA1またはOPERATE_AREA2
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
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// バッファ256Byteを初期化する
	u.us = sw;
	swdata_write_buf[0] = u.uc[0];
	swdata_write_buf[1] = u.uc[1];
	flt_WriteSector(FLT_WAVE_SWDATA_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM上の共通パラメータ運用面書き換え処理
//[]----------------------------------------------------------------------[]
///	@param[in]		運用面情報 OPERATE_AREA1またはOPERATE_AREA2
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
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// バッファ256Byteを初期化する
	u.us = sw;
	swdata_write_buf[0] = u.uc[0];
	swdata_write_buf[1] = u.uc[1];
	flt_WriteSector(FLT_PARM_SWDATA_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);

}
//[]----------------------------------------------------------------------[]
///	@brief			FROM上の音声データ運用面取得処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			運用面情報のデータ
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
///	@brief			FROM上の共通パラメータ運用面取得処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			運用面情報のデータ
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
///	@brief			FROM上の指定した運用面の音声データのバージョン、サム、レングスの取得処理
//[]----------------------------------------------------------------------[]
///	@param[in]		0:運用面1 1:運用面2
///	@param[out]		データを格納して欲しいバッファのポインタ
///	@return			void
///	@author			A.iiizumi
///	@attention		格納するデータ長はFLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_read_wave_sum_version(unsigned char sw,unsigned char *data)
{
	if(0 == sw){											// 運用面1
		Flash2Read(data, FLT_SOUND0_SUM_VER_SECTOR, FLT_SUM_VER_DATA_SIZE);
	} else {												// 運用面2
		Flash2Read(data, FLT_SOUND1_SUM_VER_SECTOR, FLT_SUM_VER_DATA_SIZE);
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			FROM上のダウンロードしたプログラムのバージョン、サム、レングスの取得処理
//[]----------------------------------------------------------------------[]
///	@param[out]		データを格納して欲しいバッファのポインタ
///	@return			void
///	@author			A.iiizumi
///	@attention		格納するデータ長はFLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_read_program_version(unsigned char *data)
{
	Flash2Read(data, FLT_PROGRAM_SUM_VER_SECTOR, FLT_SUM_VER_DATA_SIZE);
}


//[]----------------------------------------------------------------------[]
///	@brief			FROM上の指定した運用面の音声データのバージョン、サム、レングスの書き込み処理
//[]----------------------------------------------------------------------[]
///	@param[in]		0:運用面1 1:運用面2
///	@param[in]		書き込むデータを格納しているバッファのポインタ
///	@return			void
///	@author			A.iiizumi
///	@attention		格納するデータ長はFLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_write_wave_sum_version(unsigned char sw,unsigned char *data)
{
	ulong	ret;
	// アプリから書き込み用のデータとして受け取る分はFLT_PROGRAM_SUM_VER_DATA_SIZE
	// であるが、FROMには256Byte単位で書き込むのでそれはこの関数内で調整する
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// バッファ256Byteを初期化する
	memcpy(swdata_write_buf,data,FLT_SUM_VER_DATA_SIZE);// サム、バージョンをコピー

	if(0 == sw){											// 運用面1
		ret = flt_WriteSector(FLT_SOUND0_SUM_VER_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
	} else {												// 運用面2
		ret = flt_WriteSector(FLT_SOUND1_SUM_VER_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
	}
	return _FLT_RtnKind(ret);

}
//[]----------------------------------------------------------------------[]
///	@brief			FROM上のダウンロードしたプログラムのバージョン、サム、レングスの書き込み処理
//[]----------------------------------------------------------------------[]
///	@param[in]		書き込むデータを格納しているバッファのポインタ
///	@return			void
///	@author			A.iiizumi
///	@attention		格納するデータ長はFLT_SUM_VER_DATA_SIZE
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ulong FLT_write_program_version(unsigned char *data)
{
	ulong	ret;
	// アプリから書き込み用のデータとして受け取る分はFLT_PROGRAM_SUM_VER_DATA_SIZE
	// であるが、FROMには256Byte単位で書き込むのでそれはこの関数内で調整する
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// バッファ256Byteを初期化する
	memcpy(swdata_write_buf,data,FLT_SUM_VER_DATA_SIZE);// サム、バージョンをコピー
	ret = flt_WriteSector(FLT_PROGRAM_SUM_VER_SECTOR, swdata_write_buf, FLT_SWDATA_WRITE_SIZE);
	return _FLT_RtnKind(ret);
}
//[]----------------------------------------------------------------------[]
///	@brief		LOGデータ登録復電処理(FROM書き込み処理)
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
		// LOG書き込み途中での停電を保証
		if( FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG+i].req==FLT_REQ_EXEC_WRITE ){
			// 以下の処理はflt_GetWriteLogSector()実行後に停電したことを考慮してセットする
			sector = FLT_Ctrl_Backup.params.log[i].get_sector;
			seqno = FLT_Ctrl_Backup.params.log[i].get_seqno;
			switch(FLT_Ctrl_Backup.params.log[i].f_recover){
				case 0:// 前処理中
					FLT_Ctrl_Backup.params.log[i].stat_Fcount = lp->Fcount;
					FLT_Ctrl_Backup.params.log[i].stat_sectnum = logmgr->sectnum;
					FLT_Ctrl_Backup.params.log[i].f_recover = 1;// 処理開始
				case 1:// 処理開始
					// 書き込み実行
					flt_GetWriteLogSector(logmgr, &sector, &seqno);
					flt_WriteLogData(i, params,logmgr,sector, seqno);
					FLT_Ctrl_Backup.params.log[i].f_recover = 2;// 書き込み終了
				case 2:// セクタカウント更新
					if( lp->Fcount < logmgr->sectmax ){
						if(FLT_Ctrl_Backup.params.log[i].stat_Fcount == lp->Fcount){
							// 更新直前で停電したので更新する
							lp->Fcount++;
						}
						FLT_Ctrl_Backup.params.log[i].f_recover = 3;// ライトポインタ更新
					}
				case 3:// ライトポインタ更新
					logmgr->sectinfo[sector].seqno		= seqno;
					logmgr->sectinfo[sector].record_num	= params->record_num;
					break;
				case 40:// セクタ数がMAXを超えるケース
					goto point40;
				case 41:// セクタ数がMAXを超えないケース
					goto point41;
				default:// リカバリ不要とみなす
					goto point_end;
			}
			if (logmgr->sectnum + 1 > logmgr->sectmax) {
point40:
				/* 既に全セクタにデータがあったケース */
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
					// 更新直前で停電したので更新する
					logmgr->sectnum++;
				}
			}
point_end:
			LOG_DAT[i].count[FLT_Ctrl_Backup.params.log[i].sramlog_kind] = 0;// FROMに書き込んだSRAMの面をクリア
			FLT_Ctrl_Backup.req_tbl[i].req = FLT_REQ_IDLE;
		}
		FLT_Ctrl_Backup.params.log[i].f_recover = 0;// FROMの書き込み要求が無い場合はリカバリポイントをクリアする
		FLT_Ctrl_Backup.params.log[i].stat_sectnum = 0;
		FLT_Ctrl_Backup.params.log[i].stat_Fcount = 0;
		FLT_Ctrl_Backup.params.log[i].get_sector = 0;
		FLT_Ctrl_Backup.params.log[i].get_seqno = 0;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief		LOGデータの領域の書き込み要求状態チェック(書き込み要求中であるか否か)
//[]----------------------------------------------------------------------[]
///	@param[in]	kind  要求種別
///	@return		TRUE  書き込み要求済み
///				FALSE 書き込み要求なし
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
///	@brief		LOGデータの領域の要求状態を強制的にクリアする
//[]----------------------------------------------------------------------[]
///	@param[in]	kind  要求種別
///	@return		なし
///				
//[]----------------------------------------------------------------------[]
///	@author		A.iiizumi
///	@date		Create	: 2012/05/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FLT_Force_ResetLogWriteReq(short kind)
{
// NOTE:ログに対してリードまたはライトの要求によりロック状態になった時に復電時の
// ログ書き込みリカバリ処理時に要求を強制的にロック解除しないと
// flt_CheckBusy()関数でアクセス不可となりflt_LockFlash()関数から抜け出せなくなるため
	FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_LOG + kind].req = FLT_REQ_IDLE;// 要求未受付
	FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_BACKUP].req = FLT_REQ_IDLE;// 要求未受付
}
//[]----------------------------------------------------------------------[]
///	@brief		指定したアドレスの１セクタをクリアする
//[]----------------------------------------------------------------------[]
///	@param[in]	addr  消去先のアドレス
///	@return		イレース結果
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
///	@brief		FROM要求受付フラグクリア処理
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
	/* 要求受け付けを初期化 */
	for (i = 0; i < FLT_REQTBL_MAX; i++) {
		FLT_Ctrl_Backup.req_tbl[i].req		= FLT_REQ_IDLE;
		FLT_Ctrl_Backup.req_tbl[i].result	= _FLT_MkRtnVal2(FLT_NORMAL);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		LOGデータのFROM領域のセクタ数を取得する
//[]----------------------------------------------------------------------[]
///	@param[in]	id  LOGデータ種別
///	@return		セクタ数
///				-1 : LOGデータ種別不正
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
///	@brief		FROM内のログバージョンのリード処理
//[]----------------------------------------------------------------------[]
///	@param[in]	void
///	@return		FROM内のログバージョンデータ0x00～0xFF(0xFFはデータが無いことを示す)
///	            バージョン0x00と0xFFは同じ意味
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
///	@brief		FROM内のログバージョンのライト処理
//[]----------------------------------------------------------------------[]
///	@param[in]	FROMのログバージョン
///	@return		void
///	@author		A.iiizumi
///	@attention	
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/11/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
//void FLT_write_log_version(uchar log_ver)
void FLT_write_log_version(uchar log_ver,uchar chg_info_ver)
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
{
	memset(swdata_write_buf,0xFF,sizeof(swdata_write_buf));// バッファ256Byteを初期化する
	swdata_write_buf[0] = log_ver;// 先頭1Byte目に格納する
// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	// 256単位でWriteされるため、FLT_LOG_VERSION_DATA_SIZEは変えない
	swdata_write_buf[1] = chg_info_ver;// 先頭2Byte目に格納する
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
	flt_WriteSector(FLT_LOG_VERSION_SECTOR, swdata_write_buf, FLT_LOG_VERSION_DATA_SIZE);
}

// GG120600(S) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする
uchar FLT_read_chg_info_version(void)
{
	uchar ver[2];
	Flash2Read(ver, FLT_LOG_VERSION_SECTOR, sizeof(ver));
	return(ver[1]);
}
// GG120600(E) // Phase9 以前のt_prog_chg_infoからバージョンアップを行えるようにする


// MH321800(S) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
//[]----------------------------------------------------------------------[]
///	@brief		集計ログの削除判定
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		1:削除必要、0:削除不要
///	@author		Y.Tanizaki
///	@attention	
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/06/28<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar FLT_check_syukei_log_delete(void)
{
	uchar i;

	// 管理情報に変更がなければ集計ログを消さない
	if( FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].sectmax != from2_map[eLOG_TTOTAL].num ||
		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].record_size != from2_map[eLOG_TTOTAL].size ||
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].recordnum_max != LOG_SECORNUM(eLOG_TTOTAL) ||
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].sectmax != from2_map[eLOG_LCKTTL].num ||
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].record_size != from2_map[eLOG_LCKTTL].size ||
//		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].recordnum_max != LOG_SECORNUM(eLOG_LCKTTL) ) {
		FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].recordnum_max != LOG_SECORNUM(eLOG_TTOTAL) ) {
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
		return 1;
	}
	for( i = 0; i < FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].sectmax; i++ ){
		if(FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_TTOTAL].sectinfo[i].address != (i*(LogDatMax[eLOG_TTOTAL][1]+FLT_LOGOFS_RECORD) + from2_map[eLOG_TTOTAL].top)) {
			return 1;
		}
	}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
//	for( i = 0; i < FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].sectmax; i++ ){
//		if(FLT_Ctrl_NotBackup.data_info.log_mgr[eLOG_LCKTTL].sectinfo[i].address != (i*(LogDatMax[eLOG_LCKTTL][1]+FLT_LOGOFS_RECORD) + from2_map[eLOG_LCKTTL].top)) {
//			return 1;
//		}
//	}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	return 0;
}
// MH321800(E) Y.Tanizaki ICクレジット対応(集計ログを可能な限り残すようにする)
// MH810105(S) MH364301 QRコード決済対応
//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票フッタ取得
//[]----------------------------------------------------------------------[]
///	@param[out]		buf
///	@return			0x00XXXXXX	: 正常終了
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	FLT_ReadEmgFooter(char *buf)
{
	ulong ret;

	/* 本処理実行 */
	_flt_DisableTaskChg();
	ret = flt_ReadEmgFooter(buf);
	_flt_EnableTaskChg();

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票フッタ取得
//[]----------------------------------------------------------------------[]
///	@param[out]		buf : 
///	@return			0x00XXXXXX	: 正常終了
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	flt_ReadEmgFooter(char *buf)
{
	/* データ有無のチェック */
	if (!flt_CheckSum(FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EMGFOOTER, FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE)) {
		return _FLT_MkRtnVal2(FLT_NODATA);
	}
	/* データ読み込み */
	Flash2Read((uchar*)buf, FLT_PRINTDATA_SECTOR+FLT_PRINTOFS_EMGFOOTER+FLT_HEADER_SIZE, FLT_PRINTEMGFOOTER_SIZE);
	
	return _FLT_MkRtnVal2(FLT_NORMAL);
}

//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票フッタ書き込み
//[]----------------------------------------------------------------------[]
///	@param[in]		image	: プリントフッタデータ
///	@param[in]		flags	: API動作設定<br>
///							FLT_EXCLUSIVE/FLT_NOT_EXCLUSIVE	⇒ タスク初期化前か否か
///	@return			0x00XXXXXX - 正常終了
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	FLT_WriteEmgFooter(const char *image, uchar flags)
{
	ulong ret;
	
	/* 処理対象領域のロック */
	if (!flt_LockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EMGFOOTER, FLT_REQ_WRITE, flags)) {
		return _FLT_MkRtnVal2(FLT_BUSY);	/* ロックできなかった */
	}
	/* パラメータを保存 */
	FLT_Ctrl_Backup.params.emgfooter.image	= image;
	
	if (flags & FLT_EXCLUSIVE) {
		/* フラッシュタスクを使わないで書き込み */
		_flt_DisableTaskChg();

		ret = flt_WriteEmgFooter(&FLT_Ctrl_Backup.params.emgfooter,
							FLT_Ctrl_Backup.workbuf,
							&FLT_Ctrl_Backup.req_tbl[FLT_REQTBL_EMGFOOTER]);

		_flt_EnableTaskChg();
		/* 処理対象領域のロック解除 */
		flt_UnlockFlash(FLT_Ctrl_Backup.req_tbl, FLT_REQTBL_EMGFOOTER);
	}
	else {
	/* 書き込み完了は外で待つ */
		ret = _FLT_MkRtnVal(FLT_NORMAL, FLT_REQTBL_EMGFOOTER);
	}

	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票プリントフッタ書き込み
//[]----------------------------------------------------------------------[]
///	@param[in]		params	: APIユーザからの引数
///	@param[in]		wbuf	: 停電保証対応のワーク領域
///	@param[in/our]	req_tbl	: 要求受付情報
///	@return			0x00XXXXXX	: 正常終了
//[]----------------------------------------------------------------------[]
///	@author			sato.t
///	@date			Create	: 2021/12/22<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
ulong	flt_WriteEmgFooter(const t_FltParam_EmgFooter *params, uchar *wbuf, t_FltReq *req_tbl)
{
	ulong ret = _FLT_MkRtnVal2(FLT_NORMAL);
	
	/* ロゴ、ヘッダ、フッタ領域をまとめて読み出し */
	Flash2Read(wbuf, FLT_PRINTDATA_SECTOR, FLT_PRINTDATA_SECT_SIZE);
	
/* 書き込みイメージ作成 */
	/* 書き込みデータをコピー */
	memcpy(&wbuf[FLT_PRINTOFS_EMGFOOTER+FLT_HEADER_SIZE], params->image, FLT_PRINTEMGFOOTER_SIZE);
	/* SUMを作成 */
	flt_SetSum(&wbuf[FLT_PRINTOFS_EMGFOOTER], FLT_HEADER_SIZE+FLT_PRINTEMGFOOTER_SIZE);
/* 書き込むデータと比較 */
	if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
/* 書き込み実行 */
		req_tbl->req = FLT_REQ_EXEC_WRITE;		/* 停電保証のチェックポイント */
		ret = flt_WritePrintDataSector(wbuf);
		if (_FLT_RtnKind(ret) == FLT_NORMAL) {
/* ベリファイ */
			if (Flash2Cmp(FLT_PRINTDATA_SECTOR, wbuf, FLT_PRINTDATA_SECT_SIZE) != 0) {
				Flash2EraseSector(FLT_PRINTDATA_SECTOR);
				ret = _FLT_MkRtnVal2(FLT_VERIFY_ERR);
			}
		}
	}

	return ret;
}
// MH810105(E) MH364301 QRコード決済対応
