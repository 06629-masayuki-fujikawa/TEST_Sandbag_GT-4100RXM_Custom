/**********************************************************************************************************************/
/*                                                                                                                    */
/*  �֐�����          �F�U�֐��Z�d������            �L�q                                                              */
/*                                                �F--------------------------------------------------------------�F  */
/*  �֐��V���{��      �Fet96()                    �F  �U�֐��Z�d������                                            �F  */
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
// MH322914(S) K.Onodera 2016/09/13 AI-V�Ή��F�U�֐��Z
#include	"ntnet_def.h"
// MH322914(E) K.Onodera 2016/09/13 AI-V�Ή��F�U�֐��Z

void	et96()
{
	struct	REQ_RKN	*req_p;			/*  ���݌v�Z��i�[�G���A                      */
	struct	RYO_BUF_N	*r;			/*  ���݌v�Z��i�[�G���A                      */
	long	time;

	req_p		= &req_rhs[tb_number];				/*  ���ݗ����v�Z�v���|�C���^�[                */
	r			= &ryo_buf_n;						/*  ���ݗ����v�Z�v���|�C���^�[                */

	discount	= 0l;								/*  �����z�O�N���A                            */
	memmove( req_p, &req_rkn, sizeof(req_rkn) );	/*  ���ݗ����v�Z�v��TBL���v�Z����TBL�փZ�b�g  */

	cons_tax = 0;									/*  �ŋ����O�i�g�p���Ȃ��j                    */

	if (vl_frs.furikaegaku >= vl_frs.zangaku) {
		discount = vl_frs.zangaku;		// �����z
		tol_dis  += vl_frs.zangaku;		// �����z���v

		ryoukin  	 = 0;				// �����㗿��
		syu_tax  	 = 0;
		base_dis_flg = ON;

	} else  {

		discount =  vl_frs.furikaegaku;	// �����z
		ryoukin  = vl_frs.zangaku  - vl_frs.furikaegaku;	// �����㗿��
		tol_dis += discount;			// �����z���v
	}

	r->dis			= tol_dis;						/*  ���ԗ�������ł̃Z�[�u                    */
	req_p->data[2]	= discount;						/*  �����z���Z�b�g                            */
	req_p->data[3]	= ryoukin;						/*  ������z���Z�b�g                          */

	time = (vl_frs.seisan_time.Mon/10)*16 + (vl_frs.seisan_time.Mon%10);
	time <<= 8;
	time += (vl_frs.seisan_time.Day/10)*16 + (vl_frs.seisan_time.Day%10);
	time <<= 8;
	time += (vl_frs.seisan_time.Hour/10)*16 + (vl_frs.seisan_time.Hour%10);
	time <<= 8;
	time += (vl_frs.seisan_time.Min/10)*16 + (vl_frs.seisan_time.Min%10);
	req_p->data[4]	= time;							// �U�֌��E���Z�����Ƃ��Ďg�p

	return ;
}
