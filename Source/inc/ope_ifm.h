#ifndef	_OPE_IFM_H_
#define	_OPE_IFM_H_
/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO対応                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2007-03-23                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*	各種定義                                                                */
/*--------------------------------------------------------------------------*/
// 受信電文種別
enum {
	OPEIMF_RCVKIND_GENGAKU = 0,	// 減額精算
	OPEIMF_RCVKIND_FURIKAE,		// 振替精算
};

// 精算ステータス
enum {
	OPEIMF_STS_IDLE = 0,		// アイドル状態

	OPEIMF_STS_DSPRKN,			// 料金表示中

	OPEIMF_STS_GGK_RCV = 10,	// 減額精算 受信
	OPEIMF_STS_GGK_ACP,			// 減額精算 受付
	OPEIMF_STS_GGK_DONE,		// 減額精算 完了
	OPEIMF_STS_GGK_MAX,

	OPEIMF_STS_FRK_RCV = 20, 	// 振替精算 受信
	OPEIMF_STS_FRK_ACP,		 	// 振替精算 受付
	OPEIMF_STS_FRK_DONE,	 	// 振替精算 完了
	OPEIMF_STS_FRK_MAX,
};

/*--------------------------------------------------------------------------*/
/*	プロトタイプ															*/
/*--------------------------------------------------------------------------*/
void	ope_imf_Init(void);
void	ope_imf_Start(void);
BOOL	ope_imf_Pay(ushort kind, void *param);
void	ope_imf_Answer(ushort sts);
void	ope_imf_End(void);
ushort	ope_imf_GetStatus(void);
void	ope_ifm_GetFurikaeGaku(struct VL_FRS *frs);
void	ope_ifm_FurikaeCalc(uchar type);

extern	uchar FurikaeCancelFlg;
extern	uchar FurikaeMotoSts;
extern  ushort		OpeImfStatus;

#endif
