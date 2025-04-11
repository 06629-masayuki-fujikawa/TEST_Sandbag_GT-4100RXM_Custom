/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	 �G���[�o��																								   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      : S.Takahashi																				   |*/
/*| Date        : 2012-09-10																				   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/

#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"ksgmac.h"
#include	"ksgRauModem.h"
#include	"ksg_def.h"
#include	"raudef.h"

extern unsigned short		reception_level;

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���Ѵװ����/�����֐�																       |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : void RauSysErrSet()														   |*/
/*| PARAMETER1	: unsigned char		: �װ�ԍ�												   |*/
/*| PARAMETER2	: unsigned char		: 0:���� 1:����											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	RauSysErrSet( unsigned char n , unsigned char s )
{
	ulong	binData;
	char	err_mdl;

	if( s >= 2 ) {
		return;
	}
	err_mdl = ERRMDL_FOMAIBK;
	if(_is_credit_only()){			// �N���W�b�g�̂�
		err_mdl = ERRMDL_CREDIT;
	}
	switch ( n ) {
	// ����/����
	case RAU_ERR_FMA_MODEMERR_DR:										// (74)DR�M���� 3�b�o�߂��Ă�ON�ɂȂ�Ȃ�(MODEM ON��)
	case RAU_ERR_FMA_MODEMERR_CD:										// (75)CD�M����60�b�o�߂��Ă�OFF�ɂȂ�Ȃ�(MODEM OFF��)
	case RAU_ERR_FMA_PPP_TIMEOUT:										// (76)PPP�R�l�N�V�����^�C���A�E�g
	case RAU_ERR_FMA_RESTRICTION:										// (77)RESTRICTION(�K����)��M
	case RAU_ERR_FMA_NOCARRIER:											// (34)NO CARRIRE���o
	case RAU_ERR_FMA_SIMERROR:											// (35)SIM�J�[�h�O�ꌟ�m
	case RAU_ERR_FMA_ADAPTER_NG:										// FOMA�A�_�v�^�ݒ�s��
		err_chk(err_mdl, (char)n , (char)s, 0, 0);
		break;

	// ����+����
	case RAU_ERR_FMA_ANTENALOW1:										// (31)�ڑ����A���e�i���x��1���o
	case RAU_ERR_FMA_ANTENALOW2:										// (32)�ڑ����A���e�i���x��2���o
		binData = reception_level;
		err_chk2(err_mdl, (char)n , 2, 2, 0, &binData);
		break;
	case RAU_ERR_FMA_ANTENALOW:											// (79)�ڑ����A���e�i���x��0���o
		binData = reception_level;
		err_chk2(err_mdl, (char)n , 2, 2, 0, &binData);
		break;
	case RAU_ERR_FMA_REDIALOV:											// (94)���_�C�A���񐔃I�[�o�[
		err_chk(err_mdl, (char)n , 2, 0, 0);
		break;
	case RAU_ERR_FMA_MODEMPOWOFF:										// (78)MODEM �d��OFF
		// �d�g��M�󋵊m�F�ɂ��E7778�͏o�͂��Ȃ�
		if(KSG_AntAfterPppStarted == 0){
			err_chk(err_mdl, (char)n , 2, 0, 0);
		}
		break;

	// ���̑�
	default:
		break;
	}
}
