#ifndef	_OPE_IFM_H_
#define	_OPE_IFM_H_
/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO�Ή�                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2007-03-23                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*	�e���`                                                                */
/*--------------------------------------------------------------------------*/
// ��M�d�����
enum {
	OPEIMF_RCVKIND_GENGAKU = 0,	// ���z���Z
	OPEIMF_RCVKIND_FURIKAE,		// �U�֐��Z
};

// ���Z�X�e�[�^�X
enum {
	OPEIMF_STS_IDLE = 0,		// �A�C�h�����

	OPEIMF_STS_DSPRKN,			// �����\����

	OPEIMF_STS_GGK_RCV = 10,	// ���z���Z ��M
	OPEIMF_STS_GGK_ACP,			// ���z���Z ��t
	OPEIMF_STS_GGK_DONE,		// ���z���Z ����
	OPEIMF_STS_GGK_MAX,

	OPEIMF_STS_FRK_RCV = 20, 	// �U�֐��Z ��M
	OPEIMF_STS_FRK_ACP,		 	// �U�֐��Z ��t
	OPEIMF_STS_FRK_DONE,	 	// �U�֐��Z ����
	OPEIMF_STS_FRK_MAX,
};

/*--------------------------------------------------------------------------*/
/*	�v���g�^�C�v															*/
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
