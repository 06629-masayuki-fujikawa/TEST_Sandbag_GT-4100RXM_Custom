/**********************************************************************************************************************/
/*                                                                                                                    */
/*  �֐�����          �F���z���Z�d������            �L�q                                                              */
/*                                                �F--------------------------------------------------------------�F  */
/*  �֐��V���{��      �Fet95()                    �F  ���z���Z�d������                                            �F  */
/*                                                �F                                                              �F  */
/*                                                �F--------------------------------------------------------------�F  */
/*                                                �F���̓p�����[�^              �o�̓p�����[�^                    �F  */
/*                                                �F  ����                        ����                            �F  */
/*                                                �F                                                              �F  */
/*                                                �F--------------------------------------------------------------�F  */
/**********************************************************************************************************************/
#include	<string.h>
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"Tbl_rkn.h"
#include	"prm_tbl.h"
// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
#include	"ope_def.h"
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)

void	et95()
{
	struct	REQ_RKN	*req_p;			/*  ���݌v�Z��i�[�G���A                      */
	struct	RYO_BUF_N	*r;			/*  ���݌v�Z��i�[�G���A                      */
	long	wk_ryo = 0;

	req_p		= &req_rhs[tb_number];				/*  ���ݗ����v�Z�v���|�C���^�[                */
	r			= &ryo_buf_n;						/*  ���ݗ����v�Z�v���|�C���^�[                */

	discount	= 0l;								/*  �����z�O�N���A                            */
	memmove( req_p, &req_rkn, sizeof(req_rkn) );	/*  ���ݗ����v�Z�v��TBL���v�Z����TBL�փZ�b�g  */

	wk_ryo 	 = r->ryo - r->dis + r->tax;			/*  �����z������                              */
	cons_tax = 0;									/*  �ŋ����O�i�g�p���Ȃ��j                    */

// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)
//	discount = (long)(vl_ggs.waribikigaku);
	// ���u���Z�������H
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
//	if( vl_now == V_DIS_FEE ){
	if( req_rkn.param == RY_GNG_FEE ){
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
		discount = (long)(g_PipCtrl.stRemoteFee.Discount);
	}
// �s��C��(S) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
//	else if(  vl_now == V_DIS_TIM ){
	else if( req_rkn.param == RY_GNG_TIM ){
// �s��C��(E) K.Onodera 2017/01/12 �A���]���w�E(���u���Z�����̎�ʂ��������Z�b�g����Ȃ�)
		discount = (long)(g_PipCtrl.stRemoteTime.Discount);
	}
	// ���z���Z�H
	else{
		discount = (long)(vl_ggs.waribikigaku);
	}
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Z���z�w��)

	if (MACRO_WARIBIKI_GO_RYOUKIN <= discount)		/*  �����z����{�������傫���ꍇ            */
	{												/*                                            */
		discount =  MACRO_WARIBIKI_GO_RYOUKIN;		/*    ������̗��������߂�      			  */
		tol_dis  += discount;						/*  �����z���v�������z���v�{�����z            */

		ryoukin  	 = 0;							/*  ������̗��������߂�                      */
		syu_tax  	 = 0;							/*  �ŋ����O                                  */
		base_dis_flg = ON;							/*  ��{�����S�z�����t���O�n�m                */

	} else  {

		ryoukin  = wk_ryo  - discount;				/*  �����z������   ������̗��������߂�       */
		tol_dis += discount;						/*  �����z���v�������z���v�{�����z            */
	}

	r->dis			= tol_dis;						/*  ���ԗ�������ł̃Z�[�u                    */
	req_p->data[2]	= discount;						/*  �����z���Z�b�g                            */
	req_p->data[3]	= ryoukin;						/*  ������z���Z�b�g                          */
	req_p->data[4]	= cons_tax;						/*  ����Ŋz���Z�b�g                          */
													/*                                            */
	return ;
}
