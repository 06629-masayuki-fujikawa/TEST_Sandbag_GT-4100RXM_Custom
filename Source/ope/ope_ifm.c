/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO�Ή�                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2007-03-23                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"common.h"
#include	"ope_ifm.h"
#include	"ifm_ctrl.h"
#include	"prm_tbl.h"
#include	"message.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"LKmain.h"


ushort		OpeImfStatus;		/* ���z/�U�֐��Z�X�e�[�^�X�t���O */

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: ���u���Z������
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_Init(void)
{
	OpeImfStatus = OPEIMF_STS_IDLE;
	memset(&vl_ggs, 0, sizeof(vl_ggs));
	memset(&vl_frs, 0, sizeof(vl_frs));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Start
 *[]----------------------------------------------------------------------[]
 *| summary	: ���u���Z��t�J�n
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_Start(void)
{
	OpeImfStatus = OPEIMF_STS_DSPRKN;
	memset(&vl_ggs, 0, sizeof(vl_ggs));
	memset(&vl_frs, 0, sizeof(vl_frs));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Pay
 *[]----------------------------------------------------------------------[]
 *| summary	: ���z/�U�֓d����M�ʒm
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
BOOL	ope_imf_Pay(ushort kind, void *param)
{
	if (OpeImfStatus != OPEIMF_STS_DSPRKN) {
		return FALSE;	// ���ݗ����\�����łȂ����NG
	}

	// �d�����e�擾/�L���[�Z�b�g
	if (kind == OPEIMF_RCVKIND_GENGAKU) {
	// ���z���Z
		struct VL_GGS	*ggs = (struct VL_GGS*)param;
		vl_ggs = *ggs;
		OpeImfStatus = OPEIMF_STS_GGK_RCV;
		queset(OPETCBNO, IFMPAY_GENGAKU, 0, NULL);
	} else {
	// �U�֐��Z
		struct VL_FRS	*frs = (struct VL_FRS*)param;
		vl_frs = *frs;
		OpeImfStatus = OPEIMF_STS_FRK_RCV;
		queset(OPETCBNO, IFMPAY_FURIKAE, 0, NULL);
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Answer
 *[]----------------------------------------------------------------------[]
 *| summary	: ���u���Z��Ԓʒm
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_Answer(ushort sts)
{
	switch (OpeImfStatus) {
	case OPEIMF_STS_GGK_RCV:
		if (sts) {
			OpeImfStatus = OPEIMF_STS_GGK_ACP;
			IFM_Snd_GengakuAns(1);				// �v����t����
		}
		else {
			OpeImfStatus = OPEIMF_STS_DSPRKN;	// �v����M�ȑO�ɖ߂�
			IFM_Snd_GengakuAns( OPECTL.InquiryFlg ? -7:-1 );			// �v�����ۉ���
		}
		break;
	case OPEIMF_STS_GGK_ACP:
		OpeImfStatus = OPEIMF_STS_GGK_DONE;
		IFM_Snd_GengakuAns((sts)? 2 : -2);		// �������̉���
		break;
	case OPEIMF_STS_FRK_RCV:
		if (sts) {
			OpeImfStatus = OPEIMF_STS_FRK_ACP;
			IFM_Snd_FurikaeAns(1, &vl_frs);		// �v����t����
		}
		else {
			OpeImfStatus = OPEIMF_STS_DSPRKN;	// �v����M�ȑO�ɖ߂�
			IFM_Snd_FurikaeAns( (OPECTL.InquiryFlg ? -7:-1), &vl_frs);	// �v�����ۉ���
		}
		break;
	case OPEIMF_STS_FRK_ACP:
		OpeImfStatus = OPEIMF_STS_FRK_DONE;
		IFM_Snd_FurikaeAns((sts)? 2 : -2, &vl_frs);		// �������̉���
		break;
	default:
		// �����Ȃ�
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_End
 *[]----------------------------------------------------------------------[]
 *| summary	: ���u���Z��t�I��
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_End(void)
{
	switch (OpeImfStatus) {
	case OPEIMF_STS_GGK_RCV:
		// ��t����
		IFM_Snd_GengakuAns(-1);				// �v����t�O�ɐ��Z���~
		break;
	case OPEIMF_STS_GGK_ACP:
		// ���Z���~
		IFM_Snd_GengakuAns(-3);				// �v����t��ɐ��Z���~
		break;
	case OPEIMF_STS_FRK_RCV:
		// ��t����
		IFM_Snd_FurikaeAns(-1, &vl_frs);	// �v����t�O�ɐ��Z���~
		break;
	case OPEIMF_STS_FRK_ACP:
		// ���Z���~
		IFM_Snd_FurikaeAns(-3, &vl_frs);	// �v����t��ɐ��Z���~
		break;
	default:
		// ���� �� �����Ȃ�
		break;
	}
	OpeImfStatus = OPEIMF_STS_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_GetStatus
 *[]----------------------------------------------------------------------[]
 *| summary	: ���u���Z��Ԏ擾
 *| return	: 0�i�ʏ�j�A1�i���z�j�A2�i�U�ցj
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
ushort	ope_imf_GetStatus(void)
{
	if (OpeImfStatus >= OPEIMF_STS_FRK_ACP)
		return 2;		// �U�֎�t�ȍ~�Ȃ�U�֐��Z��
	if (OpeImfStatus >= OPEIMF_STS_GGK_ACP)
		return 1;		// ���z��t�ȍ~�Ȃ猸�z���Z��
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_ifm_GetFurikaeGaku
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֋��z�v�Z
 *| return	: void
 *| NOTE	: �U�֐��Z���f�[�^�|�C���^
 *[]----------------------------------------------------------------------[]*/
void	ope_ifm_GetFurikaeGaku(struct VL_FRS *frs)
{
uchar	wari_tim;

	if( OPECTL.Pay_mod == 2 ){		// �C�����Z
		wari_tim = (uchar)((prm_get(COM_PRM, S_TYP, 62, 1, 2)==2)? 1:0);
	}else{							// �U�֐��Z
		wari_tim = (uchar)prm_get(COM_PRM, S_CEN, 32, 1, 1);
	}

	frs->zangaku = ryo_buf.zankin;
	if( frs->syubetu == (char)(ryo_buf.syubet+1) ||	wari_tim == 1 ){
		frs->furikaegaku = frs->price;
	}else{
		frs->furikaegaku = frs->in_price;
	}

	if( frs->furikaegaku > frs->zangaku ){
		frs->furikaegaku = frs->zangaku;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_ifm_ExecFurikae
 *[]----------------------------------------------------------------------[]
 *| summary	: �U�֊z�v��
 *| return	: void
 *[]----------------------------------------------------------------------[]*/
void	ope_ifm_FurikaeCalc(uchar type)
{
//�U�֏������s�֐�
	vl_now = V_FRK;					// �U�֐��Z
	if( type == 1 ){				// �C�����Z�̂Ƃ�
		vl_now = V_SSS;				// �C�����Z
	}
	ope_ifm_GetFurikaeGaku(&vl_frs);
	if( type == 1 ){				// �C�����Z�̂Ƃ�
		ryo_SyuseiRyobufSet();
	}
	ryo_cal(3, OPECTL.Pr_LokNo);	// �T�[�r�X���Ƃ��Čv�Z
}
