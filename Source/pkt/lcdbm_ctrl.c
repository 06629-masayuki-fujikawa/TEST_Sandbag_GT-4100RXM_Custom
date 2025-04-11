// MH810100(S) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)

//--------------------------------------------------
//		INCLUDE
//--------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "common.h"
#include "message.h"
#include "mem_def.h"
#include "prm_tbl.h"
#include "pktctrl.h"
#include "pkt_buffer.h"
// MH810100(S) 2020/06/19 �Ԕԃ`�P�b�g���X(#4199)
#include "ntnet_def.h"
// MH810100(E) 2020/06/19 �Ԕԃ`�P�b�g���X(#4199)


//--------------------------------------------------
//		��`
//--------------------------------------------------


//--------------------------------------------------
//		�ϐ�
//--------------------------------------------------
lcdbm_ctrl_t	lcdbm_ctrl;				///< LCD���W���[���Ǘ��p���[�N

extern	ulong	Mov_cnt_dat[MOV_CNT_MAX];	// ���춳��

//--------------------------------------------------
//		�ȉ��A�֐�
//--------------------------------------------------
/**
 *	��{�ݒ艞�� ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�i�[�p���[�N�ւ̃|�C���^
 *	@date	2019/11/25(��)
 */
void lcdbm_receive_config( lcdbm_rsp_config_t *p_rcv )
{
	uchar	ist;					// ���荞�ݏ��������Call����邽�߁A���荞�݃X�e�[�^�X�ύX�����ύX

	memset( &lcdbm_ctrl.receive_data.config, 0, sizeof(lcdbm_ctrl.receive_data.config));
	memcpy( lcdbm_ctrl.receive_data.config.prgm_ver,  p_rcv->prgm_ver,  sizeof(lcdbm_ctrl.receive_data.config.prgm_ver)  );
	memcpy( lcdbm_ctrl.receive_data.config.audio_ver, p_rcv->audio_ver, sizeof(lcdbm_ctrl.receive_data.config.audio_ver) );
	lcdbm_ctrl.receive_data.config.lcd_startup_time = p_rcv->lcd_startup_time;
	lcdbm_ctrl.receive_data.config.lcd_brightness   = p_rcv->lcd_brightness;

	ist = _di2();					// ���荞�݋֎~
	if( p_rcv->lcd_startup_time <= 999999999L ){
		Mov_cnt_dat[ LCD_LIGTH_CNT ] = p_rcv->lcd_startup_time;
	}else{
		Mov_cnt_dat[ LCD_LIGTH_CNT ] = 0L;
	}
	_ei2( ist );					// ���荞�݃X�e�[�^�X�����ɖ߂�
}
// MH810100(S) 2020/06/19 �Ԕԃ`�P�b�g���X(#4199)

// �Z���^�[������ɖ��g�p�ɏ��������Ă���̂Ŏ�M��ɍX�V����i�Ȃ����[�����Łj
void Do_DiscountChecklcdbm_in_car_info(lcdbm_rsp_in_car_info_t *p_rcv)
{
	int cnt = 0;
	if(p_rcv){
// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
		if ( p_rcv->crd_info.dtReqRslt.FeeCalcOnOff ) {			// �����v�Z���ʂ���
			// �����ȊO�͍폜���đO�ɋl�߂�
			while ( cnt < ONL_MAX_DISC_NUM ) {
// GG124100(S) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6125 �yPK���ƕ��v���zNT-NET���Z�f�[�^�̊�������2001(�������z���v)���Z�b�g
// 				if ( p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu > NTNET_SECTION_WARI_MAX ) {
				if (    (p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu > NTNET_SECTION_WARI_MAX)	// �������ȊO
					 && (p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_KAIMONO_GOUKEI)		// ���׏��̔������z���v�ȊO
// GG129004(S) M.Fujikawa 2024/10/29 �������z�������Ή�
					 && (p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_KAIMONO_INFO)		// ���׏��̔����������ȊO
// GG129004(E) M.Fujikawa 2024/10/29 �������z�������Ή�
				) {
// GG124100(E) R.Endo 2022/08/04 �Ԕԃ`�P�b�g���X3.0 #6125 �yPK���ƕ��v���zNT-NET���Z�f�[�^�̊�������2001(�������z���v)���Z�b�g
					// �Ō���ȊO�͂P�O�ɏ㏑���ňړ�
					if ( cnt < (ONL_MAX_DISC_NUM - 1) ) {
						memmove(&p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt],
							&p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[(cnt + 1)],
							sizeof(p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt]) * (ONL_MAX_DISC_NUM - 1 - cnt));
					}

					// �Ō���̓N���A
					memset(&p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[(ONL_MAX_DISC_NUM - 1)],
						0, sizeof(p_rcv->crd_info.dtFeeCalcInfo.stDiscountInfo[cnt]));

					// �P�O�Ɉړ������̂ł��܂�cnt�ł��Ȃ���
					continue;
				}
				cnt++;
			}
		} else if ( p_rcv->crd_info.dtReqRslt.PayResultInfo ) {	// ������񂠂�
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)

// MH810100(S) 2020/08/04 �d�l�ύX #4573�Z���^�[�����M���������X�e�[�^�X��ύX���Ȃ��悤�ɂ���
// 		for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
// 			// ��������(���z=6) or ��������(����=106)
// 			if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_AMT || 
// 				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_TIME ){
				
// 				if(p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].UsedDisc != 0){
// 					// �g�p�ς݊����i���z/���ԁj�������Ă�����g�p�ς݁�2�ɋ����I�ɕς���
// 					p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscStatus = 2; 
// 				} 
// 			}
// // MH810100(S) 2020/07/17 �d�l�ύX #4553�y�A���s��w�E�����z�@���������Ɠ��l�ɁA���̃I�����C�������ł��������i�����X�e�[�^�X0�j�ɑΉ����Ăق���
// 			// 002�i�X����[���z]�j�܂���102�i�X����[����]�j
// 			else if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_M || 
// 				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_T ){
// 				if(p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].UsedDisc != 0 && p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscInfo != 0){
// 					// �g�p�ς݊����i���z/���ԁj�������Ă����� ���� �������000�ȊO�̏ꍇ �g�p�ς݁�2�ɋ����I�ɕς���
// 					p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscStatus = 2; 
// 				}
// 			}
// // MH810100(E) 2020/07/17 �d�l�ύX #4553�y�A���s��w�E�����z�@���������Ɠ��l�ɁA���̃I�����C�������ł��������i�����X�e�[�^�X0�j�ɑΉ����Ăق���
//		}
// MH810100(E) 2020/08/04 �d�l�ύX #4573�Z���^�[�����M���������X�e�[�^�X��ύX���Ȃ��悤�ɂ���
// MH810100(S) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)
		//�E���������i6/106)�E�T�[�r�X��(1/101)�E�X����(2/102)�ȊO�̏ꍇ�́ARXM��M����0�N���A����B
		//�E���������i6/106)�E�T�[�r�X��(1/101)�E�X����(2/102)�̏ꍇ�́ARXM��M���ɋ��z/���Ԃ�0�N���A����
		for( cnt=0; cnt<ONL_MAX_DISC_NUM;  ){
			if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_AMT || 
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SHOP_DISC_TIME ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_M ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_KAK_T ||
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(���ɏ�񊄈��`�F�b�N�̕ύX)
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_TKAK_M ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_TKAK_T ||
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(���ɏ�񊄈��`�F�b�N�̕ύX)
// MH810100(S) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
// 50/150�������͒ʉ߂����Ă���
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SYUBET_TIME ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SYUBET ||
// MH810100(E) 2020/09/07 #4813 �y���؉ێw�E�����z��ʊ�����t�^�����ꍇ�Ƀ��A���^�C���ʐM�̐��Z�f�[�^�Ɏ�ʊ������i�[����Ȃ��iNo.81�j
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SVS_M ||
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu == NTNET_SVS_T ){

				//�E���������i6/106)�E�T�[�r�X��(1/101)�E�X����(2/102)�̏ꍇ�́ARXM��M���ɋ��z/���Ԃ�0�N���A����
				p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].Discount = 0;
			}else{
				if( cnt < ONL_MAX_DISC_NUM ){	//
					if( p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu){ 
						if( cnt < (ONL_MAX_DISC_NUM -1 ) ){	//�Ō���̏ꍇ��memmove���Ȃ�
							// �O�ɂP�㏑���ňړ�
							memmove(&p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt],&p_rcv->crd_info.dtCalcInfo.stDiscountInfo[cnt+1],sizeof(stDiscount2_t)*(ONL_MAX_DISC_NUM-cnt-1));
						}
						// �Ō�����N���A���Ă���
						memset(&p_rcv->crd_info.dtCalcInfo.stDiscountInfo[ONL_MAX_DISC_NUM-1],0,sizeof(stDiscount2_t));
						// 1�O�Ɉړ������̂ł��܂�cnt�ł��Ȃ���
						continue;
					}else{
						// ��l�߂���Ă���͂��Ȃ̂ŋ�ɂȂ�����I��
						break;
					}
				}

			}
			cnt++;
		}
// MH810100(E) 2020/07/29 �Ԕԃ`�P�b�g���X(#4561 �Z���^�[�����M������������ύX����)

// GG124100(S) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
		}
// GG124100(E) R.Endo 2022/07/21 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�Z���^�[�⍇��Rev.No.2)
	}

}
// MH810100(E) 2020/06/19 �Ԕԃ`�P�b�g���X(#4199)

/**
 *	���ɏ�� ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@date	2019/11/25(��)
 */
void lcdbm_receive_in_car_info( lcdbm_rsp_in_car_info_t *p_rcv )
{
	memset( &lcdbm_rsp_in_car_info_recv, 0, sizeof(lcdbm_rsp_in_car_info_t));
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:541)�Ή�
//	memcpy( &lcdbm_rsp_in_car_info_recv, &p_rcv->command.id, sizeof(lcdbm_rsp_in_car_info_t)  );
	memcpy( &lcdbm_rsp_in_car_info_recv, p_rcv, sizeof(lcdbm_rsp_in_car_info_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:541)�Ή�
// MH810100(S) 2020/06/19 �Ԕԃ`�P�b�g���X(#4199)
	Do_DiscountChecklcdbm_in_car_info(&lcdbm_rsp_in_car_info_recv);
// MH810100(E) 2020/06/19 �Ԕԃ`�P�b�g���X(#4199)

	queset( OPETCBNO, LCD_IN_CAR_INFO, 0, NULL );
}

/**
 *	QR�f�[�^ ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@date	2019/11/27(��)
 */
void lcdbm_receive_QR_data( lcdbm_rsp_QR_data_t *p_rcv )
{
	memset( &lcdbm_rsp_QR_data_recv, 0, sizeof(lcdbm_rsp_QR_data_t));
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:542)�Ή�
//	memcpy( &lcdbm_rsp_QR_data_recv, &p_rcv->command.id, sizeof(lcdbm_rsp_QR_data_t)  );
	memcpy( &lcdbm_rsp_QR_data_recv, p_rcv, sizeof(lcdbm_rsp_QR_data_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:542)�Ή�
	queset( OPETCBNO, LCD_QR_DATA, 0, NULL );
}

// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
/**
 *	���[�����j�^�f�[�^���� ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@param[out]	id		ID�ւ̃|�C���^
 *	@param[out]	result	���ʂւ̃|�C���^
 *	@date	2023/01/19(��)
 */
void lcdbm_receive_DC_LANE_res( lcdbm_rsp_LANE_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j

/**
 *	���Z�����f�[�^ ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@param[out]	id		ID�ւ̃|�C���^
 *	@param[out]	result	���ʂւ̃|�C���^
 *	@date	2019/11/27(��)
 */
void lcdbm_receive_RTPay_res( lcdbm_rsp_pay_data_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
/**
 *	�̎��؃f�[�^���� ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@param[out]	id		ID�ւ̃|�C���^
 *	@param[out]	result	���ʂւ̃|�C���^
 *	@date	2024/10/25
 */
void lcdbm_receive_RTReceipt_res( lcdbm_rsp_receipt_data_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

/**
 *	QR�m��E��������f�[�^ ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@param[out]	id		ID�ւ̃|�C���^
 *	@param[out]	result	���ʂւ̃|�C���^
 *	@date	2019/11/27(��)
 */
void lcdbm_receive_DC_QR_res( lcdbm_rsp_QR_conf_can_res_t *p_rcv, ulong *id, uchar *result )
{
	*id		= p_rcv->id;
	*result	= p_rcv->result;
}

/**
 *	QR���[�_���䉞�� ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@date	2019/11/25(��)
 */
void lcdbm_receive_QR_ctrl_res( lcdbm_rsp_QR_ctrl_res_t *p_rcv )
{
	memset( &LcdRecv.lcdbm_rsp_QR_ctrl_res, 0, sizeof(lcdbm_rsp_QR_ctrl_res_t) );
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:543)�Ή�
//	memcpy( &LcdRecv.lcdbm_rsp_QR_ctrl_res, &p_rcv->command.id, sizeof(lcdbm_rsp_QR_ctrl_res_t)  );
	memcpy( &LcdRecv.lcdbm_rsp_QR_ctrl_res, p_rcv, sizeof(lcdbm_rsp_QR_ctrl_res_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:543)�Ή�
	queset( OPETCBNO, LCD_MNT_QR_CTRL_RESP, 0, NULL );
}

/**
 *	QR�ǎ挋�� ��M������
 *
 *	@param[in]	p_rcv	��M�f�[�^�o�b�t�@�ւ̃|�C���^
 *	@date	2019/11/25(��)
 */
void lcdbm_receive_QR_rd_rslt( lcdbm_rsp_QR_rd_rslt_t *p_rcv )
{
	memset( &LcdRecv.lcdbm_rsp_QR_rd_rslt, 0, sizeof(lcdbm_rsp_QR_rd_rslt_t));
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:544)�Ή�
//	memcpy( &LcdRecv.lcdbm_rsp_QR_rd_rslt, &p_rcv->command.id, sizeof(lcdbm_rsp_QR_rd_rslt_t)  );
	memcpy( &LcdRecv.lcdbm_rsp_QR_rd_rslt, p_rcv, sizeof(lcdbm_rsp_QR_rd_rslt_t)  );
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:544)�Ή�
	queset( OPETCBNO, LCD_MNT_QR_READ_RESULT, 0, NULL );
}

/**
 *	LCD���䃂�W���[���\�t�g�E�F�A�̃o�[�W��������Ԃ�
 *
 *	@param[out]	dst		�o�[�W���������i�[���邽�߂̔z��ւ̃|�C���^
 *	@param[in]	size	�ő�T�C�Y
 *	@return				�o�[�W�����f�[�^���i�o�C�g���j
 *	@date	2019/11/14
 *	@note
 *	-	LCD���W���[������̊�{�ݒ艞���Ŏ擾�ς݂̃f�[�^��Ԃ��B
 *	-	�����Ŏ������z��Ɋi�[����o�[�W�����f�[�^�ɂ͏I�[�����͊܂܂Ȃ��B
 *	-	�Ԓl�Ŏ������o�[�W�����f�[�^���ɂ͏I�[�����͊܂܂Ȃ��B
 */
uchar lcdbm_get_config_prgm_ver(char *dst, uchar size)
{
	uchar	*src = &lcdbm_ctrl.receive_data.config.prgm_ver[0];
	uchar	loop_max;
	uchar	count;

	loop_max = sizeof(lcdbm_ctrl.receive_data.config.prgm_ver);
	if (loop_max > size ) {
		loop_max = size;
	}
	for ( count = 0; count < loop_max; count++ ) {
		if ( *src == '\0' ) {
			break;
		}
		*dst++ = *src++;
	}
	return count;
}

/**
 *	�����f�[�^�̃o�[�W��������Ԃ�
 *
 *	@param[out]	dst		�o�[�W���������i�[���邽�߂̔z��ւ̃|�C���^
 *	@param[in]	size	�ő�T�C�Y
 *	@return				�o�[�W�����f�[�^���i�o�C�g���j
 *	@date	2019/11/14
 *	@note
 *	-	LCD���W���[������̊�{�ݒ艞���Ŏ擾�ς݂̃f�[�^��Ԃ��B
 *	-	�����Ŏ������z��Ɋi�[����o�[�W�����f�[�^�ɂ͏I�[�����͊܂܂Ȃ��B
 *	-	�Ԓl�Ŏ������o�[�W�����f�[�^���ɂ͏I�[�����͊܂܂Ȃ��B
 */
uchar lcdbm_get_config_audio_ver(char *dst, uchar size)
{
	uchar	*src = &lcdbm_ctrl.receive_data.config.audio_ver[0];
	uchar	loop_max;
	uchar	count;

	loop_max = sizeof(lcdbm_ctrl.receive_data.config.audio_ver);
	if (loop_max > size ) {
		loop_max = size;
	}
	for ( count = 0; count < loop_max; count++ ) {
		if ( *src == '\0' ) {
			break;
		}
		*dst++ = *src++;
	}
	return count;
}

/**
 *	LCD���W���[���N�����Ԃ�Ԃ�
 *
 *	@param[in]	none
 *	@param[out]	none
 *	@return				LCD���W���[���N������
 *	@date	2019/11/14
 *	@note
 *	-	LCD���W���[������̊�{�ݒ艞���Ŏ擾�ς݂̃f�[�^��Ԃ��B
 */
ulong lcdbm_get_config_lcd_startup_time(void)
{
	return(lcdbm_ctrl.receive_data.config.lcd_startup_time);
}

/**
 *	LCD�P�x��Ԃ�
 *
 *	@param[in]	none
 *	@param[out]	none
 *	@return				LCD�P�x
 *	@date	2019/11/14
 *	@note
 *	-	LCD���W���[������̊�{�ݒ艞���Ŏ擾�ς݂̃f�[�^��Ԃ��B
 */
ushort lcdbm_get_config_lcd_brightness(void)
{
	return(lcdbm_ctrl.receive_data.config.lcd_brightness);
}

/**
 *	�擾������{�ݒ艞���̐擪�|�C���^��Ԃ�
 *
 *	@param[in]	none
 *	@param[out]	none
 *	@return				��{�ݒ艞���̐擪�|�C���^
 *	@date	2019/11/14
 *	@note
 *	-	LCD���W���[������̊�{�ݒ艞���Ŏ擾�ς݂̃f�[�^��Ԃ��B
 */
uchar* lcdbm_get_config_ptr(void)
{
	return((uchar*)&lcdbm_ctrl.receive_data.config);
}

/**
 *	��M�f�[�^����͂���
 *
 *	@param[in,out]	p			��M�f�[�^�i�[�p���[�N�ւ̃|�C���^
 *	@param[in,out]	data_length	��M�f�[�^��
 *	@return	��M�f�[�^�̂h�c�i�R�}���h�h�c�ƃT�u�R�}���h�h�c�����Z�����S�o�C�g�l�j
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@date	2009/12/08(��)
 *	@note
 *	-	�p�P�b�g�ʐM�^�X�N���Ǘ�����o�b�t�@�����M�f�[�^�����o��
 */
unsigned long lcdbm_analyze_packet_event( lcdbm_rsp_work_t *p, unsigned short *data_length )
{
	unsigned long	id;

	*data_length = PKTbuf_ReadRecvCommand( (unsigned char *)p );
	id = lcdbm_command_id( p->command.id, p->command.subid );

	return id;
}

//[]----------------------------------------------------------------------[]
///	@brief			�R�}���h���b�Z�[�W�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		data	: �R�}���h(�R�}���h���T�u�R�}���h)
///	@return			ope�^�X�N�ɓn�����b�Z�[�W�ԍ�
///	@attention		���̂܂�ope�^�X�N�ɓn���Ȃ����̂�0��Ԃ��B
///	@note			�R�}���h�ɑΉ����郁�b�Z�[�W�ԍ���Ԃ��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/13<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort	lcdbm_check_message(const ulong data)
{
	ushort	msg;

	switch (data) {		// �R�}���h(2byte) + �T�u�R�}���h(2byte)
		case LCDBM_RSP_NOTIFY_CONFIG:		// ��{�ݒ艞��(ptk�^�X�N���ŏ��ۑ�)
		case LCDBM_RSP_TENKEY_KEYDOWN:		// �e���L�[�������(�������b�Z�[�W�ԍ��ɕϊ�)
			msg = 0;
			break;
		case LCDBM_RSP_ERROR:				// �G���[�ʒm
			msg = LCD_ERROR_NOTICE;
			break;
		case LCDBM_RSP_NOTICE_OPE:			// ����ʒm
			msg = 0;
			break;
		case LCDBM_RSP_IN_INFO:				// ���ɏ��
			msg = 0;
			break;
		case LCDBM_RSP_QR_CONF_CAN_RES:		// QR�m��E����f�[�^����(ptk�^�X�N���ŏ���)
			msg = 0;
			break;
		case LCDBM_RSP_QR_DATA:				// QR�f�[�^
			msg = 0;
			break;
		case LCDBM_RSP_PAY_DATA_RES:		// ���Z�����f�[�^(ptk�^�X�N���ŏ���)
			msg = 0;
			break;
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case LCDBM_RSP_RECEIPT_DATA_RES:	// �̎��؃f�[�^����
			msg = 0;
			break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		case LCDBM_RSP_LANE_RES:			// ���[�����j�^����
			msg = 0;
			break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		case LCDBM_RSP_MNT_QR_CTRL_RES:		// QR���[�_���䉞��
			msg = 0;
			break;
		case LCDBM_RSP_MNT_QR_RD_RSLT:		// QR�ǎ挋��
			msg = 0;
			break;
		case LCDBM_RSP_MNT_RT_CON_RSLT:		// ���A���^�C���ʐM�a�ʌ���
			msg = LCD_MNT_REALTIME_RESULT;
			break;
		case LCDBM_RSP_MNT_DC_CON_RSLT:		// DC-NET�ʐM�a�ʌ���
			msg = LCD_MNT_DCNET_RESULT;
			break;
		default:							// �s���R�}���h��M
			msg = 0;
			break;
	}

	return(msg);
}

//[]----------------------------------------------------------------------[]
///	@brief			�L�[�v�A���C�u���g���C��ԃ`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		cnt	:�L�[�v�A���C�u���g���C��(=�ݒ臂48-0020)
///	@return			TRUE:���g���COK/FALSE:���g���C�I�[�o
///	@attention
///	@note
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/13<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
BOOL	lcdbm_check_keepalive_status(uchar cnt)
{
	if (lcdbm_ctrl.status.keepalive++ > cnt) {
		// �L�[�v�A���C�u���g���C�I�[�o
		return(FALSE);
	}

	return(TRUE);
}

//[]----------------------------------------------------------------------[]
///	@brief			�L�[�v�A���C�u���g���C�J�E���^���Z�b�g
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
///	@attention
///	@note
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/13<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	lcdbm_reset_keepalive_status(void)
{
	lcdbm_ctrl.status.keepalive = 0;
}

//@debug <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//--------------------------------------------------
//		�e�X�g�R�[�h
//--------------------------------------------------
#ifdef	CRW_DEBUG

void test_lcdbm_function( void )
{
//-#if 0
//-//
//-//	lcdbm_reset_audio_No();				// [0]=0,[1]=0,[2]=0,[3]=0,[4]=0
//-//
//-//	// �ǉ��e�X�g						// ���L�̊֐����s���lcdbm_ctrl.audio.issued_buf[]�̒l
//-//	lcdbm_push_audio_No( 1 );			// [0]=1
//-//	lcdbm_push_audio_No( 10 );			// [0]=1,[1]=10
//-//	lcdbm_push_audio_No( 100 );			// [0]=1,[1]=10,[2]=100
//-//	lcdbm_push_audio_No( 1000 );		// [0]=1,[1]=10,[2]=100,[3]=1000
//-//	lcdbm_push_audio_No( 10000 );		// [0]=1,[1]=10,[2]=100,[3]=1000,[4]=10000
//-//	lcdbm_push_audio_No( 65535 );		// [0]=10,[1]=100,[2]=1000,[3]=10000,[4]=65535	�i�ŌẪf�[�^��ǂ��o���j
//-//
//-//	// �폜�e�X�g
//-//	lcdbm_pop_audio_No( 10 );			// [0]=100,[1]=1000,[2]=10000,[3]=65535,[4]=0
//-//	lcdbm_pop_audio_No( 20 );			// [0]=100,[1]=1000,[2]=10000,[3]=65535,[4]=0	�i�Y���f�[�^���Ȃ��ꍇ�͉������Ȃ��j
//-//	lcdbm_pop_audio_No( 1000 );			// [0]=100,[1]=10000,[2]=65535,[3]=0,[4]=0
//-//	lcdbm_pop_audio_No( 20000 );		// [0]=100,[1]=10000,[2]=65535,[3]=0,[4]=0
//-//	lcdbm_pop_audio_No( 65535 );		// [0]=100,[1]=10000,[2]=0,[3]=0,[4]=0
//-//
//-//	// �ŌÃf�[�^�������e�X�g
//-//	lcdbm_delete_oldest_audio_No();		// [0]=100,[1]=10000,[2]=0,[3]=0,[4]=0		�i�o�b�t�@�����t�łȂ��ꍇ�͉������Ȃ��j
//-//	lcdbm_push_audio_No( 1 );			// [0]=100,[1]=10000,[2]=1,[3]=0,[4]=0
//-//	lcdbm_delete_oldest_audio_No();		// [0]=100,[1]=10000,[2]=1,[3]=0,[4]=0		�i�o�b�t�@�����t�łȂ��ꍇ�͉������Ȃ��j
//-//	lcdbm_push_audio_No( 10 );			// [0]=100,[1]=10000,[2]=1,[3]=10,[4]=0
//-//	lcdbm_delete_oldest_audio_No();		// [0]=100,[1]=10000,[2]=1,[3]=10,[4]=0		�i�o�b�t�@�����t�łȂ��ꍇ�͉������Ȃ��j
//-//	lcdbm_push_audio_No( 2000 );		// [0]=100,[1]=10000,[2]=1,[3]=10,[4]=2000
//-//	lcdbm_delete_oldest_audio_No();		// [0]=10000,[1]=1,[2]=10,[3]=2000,[4]=0	�i�ŌẪf�[�^�������j
//-//
//-//	// �S���N���A����e�X�g
//-//	lcdbm_push_audio_No( 65535 );		// [0]=10000,[1]=1,[2]=10,[3]=2000,[4]=65535
//-//	lcdbm_reset_audio_No();				// [0]=0,[1]=0,[2]=0,[3]=0,[4]=0
//-//	lcdbm_delete_oldest_audio_No();		// [0]=0,[1]=0,[2]=0,[3]=0,[4]=0	�i�o�b�t�@�����t�łȂ��ꍇ�͉������Ȃ��j
//-//
//-#endif
}

#endif//CRW_DEBUG
//@debug >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// MH810100(E) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
