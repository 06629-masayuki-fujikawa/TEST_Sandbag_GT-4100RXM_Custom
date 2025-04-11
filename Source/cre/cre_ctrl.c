//[]-----------------------------------------------------------------------[]
//| System      : FT4000													|
//| Module      : �N���W�b�g����֐��Q										|
//| 			  OPETASK ����֐��R�[�������B							|
//[]-----------------------------------------------------------------------[]
//| Date        : 2013-07-01												|
//| Update      :															|
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"LKmain.h"
#include	"flp_def.h"
#include	"oiban.h"
#include	"common.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"lcd_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"
#include	"raudef.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"
#include	"aes_sub.h"
#include	"ksg_def.h"

// MH810103 GG119202(S) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
////
//// ����G���A���̐錾
////
//td_cre_ctl			cre_ctl;			// ����G���A
//td_creSeisanInfo	creSeisanInfo;		// ���Z���
//extern CRE_SALENG	cre_saleng_work;	// ���㋑���ް��P�O�����ҏW�p���[�N
//
//static	ulong	creConvAtoB( uchar *asc_buf, ushort len );
//
//
//#define	ON_OFF	2
//
////[]-----------------------------------------------------------------------[]
////| �N���W�b�gOPE���䃁�C��													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creCtrl( event )											|
////| PARAMETER    : event : �N���v��											|
////| RETURN VALUE : �N���v���ʂ̖߂�l,�߂�l�ɈӖ��̖������̂� 0 �Œ�		|
////|				   creSeisanInfo = ���Z���									|
////|				   cre_ctl.RcvData = ��M�f�[�^								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creCtrl( short event )
//{
//	short	ret = 0;
//
//	cre_ctl.Event = event;
//	switch( event ){
//	case	CRE_EVT_SEND_OPEN:			// �J�ǃR�}���h(01)
//		ret = creSendData_OPEN();
//		break;
//	case	CRE_EVT_SEND_CONFIRM:		// �^�M�⍇���f�[�^���M�v��(03)
//		ret = creSendData_CONFIRM();
//		break;
//	case	CRE_EVT_SEND_SALES:			// ����˗��f�[�^���M�v��(05)
//		ret = creSendData_SALES();
//		break;
//	case	CRE_EVT_SEND_ONLINETEST:	// ��ײ�ýđ��M�v��(07)
//		ret = creSendData_ONLINETEST();
//		break;
//	case	CRE_EVT_SEND_RETURN:		// �ԕi�⍇���f�[�^���M�v��(09)
//		ret = creSendData_RETURN();
//		break;
//	case	CRE_EVT_TIME_UP:			// ��ѱ��
//		ret = creTimeOut();
//		break;
//	case	CRE_EVT_RECV_ANSWER:		// ������M
//		ret = creRecvData();
//		break;
//	default:
//		break;
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| �N���W�b�g����G���A������												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creCtrlInit( uchar flg )									|
////| PARAMETER    : uchar	flg	: 1=�[�������ʔԂ�����������				|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creCtrlInit( uchar flg )
//{
//	if( flg == 1 ) {
//		cre_slipno = 0;								// �[�������ʔ�(�g�p�O�ɉ��Z�����̂ŁA�����l��0�Ƃ���)
//	}
//	
//	memset( &cre_ctl, 0, sizeof(cre_ctl) );
//
//	// �J�ǃR�}���h���g���C����f�[�^������
//	creOpen_Init();
//
//	// ����˗��f�[�^���g���C����f�[�^������
//	creSales_Init();
//
//	// �����m�F�̐���f�[�^������
//	creOnlineTest_Init();
//
//	// �����f�[�^��M�^�C���A�E�g�i�b�j
//	cre_ctl.Result_Wait = (uchar)prm_get( COM_PRM, S_CRE, 5, 3, 1 );	// �o���@�̐����l�F10�b
//	if( cre_ctl.Result_Wait < 1 || cre_ctl.Result_Wait > 99 ){
//		cre_ctl.Result_Wait = 10;					// �ݒ�l���͈͊O�̏ꍇ�͂P�O�b�Ƃ���
//	}
//	
//	if( cre_saleng.ken >= CRE_SALENG_MAX ) {
//		cre_ctl.Credit_Stop = 1;					// ���㋑�ۃf�[�^������̏ꍇ�̓N���W�b�g���Z�s��
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| �N���W�b�g���Z�^�s�`�F�b�N											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creStatusCheck											|
////| PARAMETER    : void														|
////| RETURN VALUE : short													|
////|					0:���Z�� / 1:�ʐM�� / 2:������W������					|
////|					-1:�ڼޯĻ��ް�ڑ��Ȃ� / -2:��~�� / -3:���J��			|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]-----------------------------------------------------------------------[]
////| �N���W�b�g�T�[�o�[�֐��Z�v�����\���`�F�b�N����  						|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creStatusCheck( void )
//{
//	uchar	retsts = CRE_STATUS_OK;
//
//	if( ! CREDIT_ENABLED() ){
//		retsts |= CRE_STATUS_DISABLED;		//�ڑ��Ȃ�.or.�C�j�V����������
//	}
//	if( cre_ctl.Initial_Connect_Done == 0 ){
//		retsts |= CRE_STATUS_NOTINIT;		//����ڑ�������
//	}
//	if( cre_ctl.Credit_Stop ){
//		retsts |= CRE_STATUS_STOP;			//��~��
//	}
//	if( cre_uriage.UmuFlag == ON ){
//		retsts |= CRE_STATUS_UNSEND;		//����˗�(05)�����M
//	}
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		retsts |= CRE_STATUS_SENDING;		//�ʐM������
//	}
//	if( KSG_gPpp_RauStarted == 0 ) {		//PPP���ڑ�
//		retsts |= CRE_STATUS_PPP_DISCONNECT;
//	}
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| ���Z���z�̌��x�z�ȉ����`�F�b�N����										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creLimitCheck											|
////| PARAMETER    : Amount : ���Z���悤�Ƃ��Ă�����z						|
////| RETURN VALUE : 0 : �ȉ� / -1 : �����Ă���								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creLimitCheck( ulong Amount )
//{
//	ulong	Limit;
//	Limit = (ulong)prm_get( COM_PRM, S_CRE, 2, 6, 1 );
//
//	if( Limit != 0 && Amount > Limit ){
//		return -1;
//	}
//	return 0;
//}
//
////[]-----------------------------------------------------------------------[]
////| �P����������															|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creMinCyclCheckProc   	                           		|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]-----------------------------------------------------------------------[]
////| ��ײ�ýĂ𑗐M����K�v������ꍇ�A���M����								|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOneMinutesCheckProc( void )
//{
//	short	do_flag = 0;
//
//	if( ! CREDIT_ENABLED() ){
//		return;		// ���ڑ�.or.�ڑ��Ȃ�
//	}
//
//	// �����m�F�̌o�ߎ��Ԃ��J�E���g
//	creOnlineTest_Count();
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return; 	// �ʐM������
//	}
//	// ����ڑ�������
//	if( cre_ctl.Initial_Connect_Done == 0 ){
//		// �J�ǃR�}���h�̌o�ߎ��Ԃ��`�F�b�N
//		if( creOpen_Check() != 0 ){
//			do_flag = 1;
//		}
//		goto LSEND_CHECK;	// ����ڑ���������܂ł͈ȍ~�̃`�F�b�N�͕s�v
//	}
//	// ����˗�����(06)�f�[�^����M���i05���M���j
//	if( cre_uriage.UmuFlag == ON ){
//		// ����˗��f�[�^�̌o�ߎ��Ԃ��`�F�b�N
//		if( creSales_Check() != 0 ){
//			do_flag = 2;
//		}
//		goto LSEND_CHECK;
//	}
//	// �����m�F�̌o�ߎ��Ԃ��`�F�b�N
//	if( creOnlineTest_Check() != 0 ){
//		do_flag = 3;
//		goto LSEND_CHECK;
//	}
//
//LSEND_CHECK:
//	switch( do_flag ){
//	case 1:
//		creOpen_Send();				// �J�ǃR�}���h(01)���M
//		break;
//	case 2:
//		creSales_Send(CRE_KIND_RETRY);
//		break;
//	case 3:
//		creOnlineTest_Send();		// �����m�F(07)���M
//		break;
//	default:
//		break;
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ��ײ�ýđ��M����														|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTestCheck										|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : short : 1:���M�����t 1�ȊO:���M�s��/���s				|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]-----------------------------------------------------------------------[]
////| հ�ް��Á@�g���@�\�@�ڼޯĐڑ��m�F���ɌĂ΂��							|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creOnlineTestCheck( void )
//{
//	if( ! CREDIT_ENABLED() ){
//		return -1;		// ���ڑ�.or.�ڑ��Ȃ�
//	}
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2; 		// �ʐM������
//	}
//
//	cre_ctl.OpenKind = CRE_KIND_MANUAL;		// �J�ǔ����v���Ɏ蓮��ݒ�
//	return creCtrl( CRE_EVT_SEND_OPEN );	// �J�ǃR�}���h(01)
//}
//
////[]-----------------------------------------------------------------------[]
////| �N���W�b�g��~�v���G���[�`�F�b�N										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creErrorCheck											|
////| RETURN VALUE : 0:�Ȃ� / 1:����˗��̂�, 2:���㋑�ۂ̂�, 3:����			|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creErrorCheck( void )
//{
//	short	ret = 0;
//
//	// �N���W�b�g��~��
//	if( cre_ctl.Credit_Stop ){
//
//		// �v���F����˗������M
//		if( cre_uriage.UmuFlag == ON ){
//			ret += 1;
//		}
//
//		// �v���F���㋑�ۃf�[�^�t��
//		if( cre_saleng.ken >= CRE_SALENG_MAX ){
//			ret += 2;
//		}
//
//		// �v���F�I�����C���e�X�g�ŃT�[�o�[���v���I�G���[����M�i���Ή��j
//		;	// �Y�������Ȃ�
//
//	}
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| 01 ���M����																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_OPEN											|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_OPEN( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// �ʐM��
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// ���j�^�o��
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_OPEN, (ulong)cre_ctl.OpenKind);
//
//	//���M�f�[�^�ҏW�`���M�L���[�Ɋi�[
//	retsts = (NTNET_Snd_Data136_01( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_OPEN;		// ��ԍX�V�i02�҂��j
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 03 ���M����																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_CONFIRM										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
////short	creSendData_DATASEND( void )
//short	creSendData_CONFIRM( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// �ʐM��
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// ���j�^�o��
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_CONFIRM, 0);
//
//	//���M�f�[�^�ҏW�`���M�L���[�Ɋi�[
//	retsts = (NTNET_Snd_Data136_03() ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_CONFIRM;	// ��ԍX�V�i04�҂��j
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 05 ���M����																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_SALES										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_SALES( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// �ʐM��
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// ���j�^�o��
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_SALES, (ulong)cre_ctl.SalesKind);
//
//	// ���M�f�[�^�ҏW�`���M�L���[�Ɋi�[
//	retsts = (NTNET_Snd_Data136_05( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_SALES;	// ��ԍX�V�i06�҂��j
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 07 ���M����																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_ONLINETEST									|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_ONLINETEST( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// �ʐM��
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// ���j�^�o��
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_ONLINETEST, 0);
//
//	// ���M�f�[�^�ҏW�`���M�L���[�Ɋi�[
//	retsts = (NTNET_Snd_Data136_07( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_ONLINETEST;	// ��ԍX�V�i08�҂��j
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 09 ���M����																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_RETURN										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_RETURN( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// �ʐM��
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// ���j�^�o��
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_RETURN, 0);
//
//	// ���M�f�[�^�ҏW�`���M�L���[�Ɋi�[
//	retsts = (NTNET_Snd_Data136_09( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_RETURN;	// ��ԍX�V�i0A�҂��j
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| ������M����															|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creRecvData												|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creRecvData( void )
//{
//	short retsts = 1;
//	DATA_KIND_137_02	*Recv137_02;
//	DATA_KIND_137_04	*Recv137_04;
//	DATA_KIND_137_06	*Recv137_06;
//	DATA_KIND_137_08	*Recv137_08;
//	DATA_KIND_137_0A	*Recv137_0A;
//	short	ReSendFlg = 0;
//	short	kind = 0;					// ���M�v��
//	ulong	result = 0;					// ���ʇ@�A
//	short	tout_flg = 0;				// �^�C���A�E�g�ʒm����^�Ȃ�
//
//	creOnlineTest_Reset();	// ���ʐM���ԃ��Z�b�g
//	err_chk2( ERRMDL_CREDIT, ERR_CREDIT_CONNECTCHECK_ERR, 0x00, 0, 0, NULL );	// �G���[(����)
//
//	cre_ctl.RcvData = Cre_GetRcvNtData();
//
//	Recv137_02 = (DATA_KIND_137_02 *)cre_ctl.RcvData;
//	Recv137_04 = (DATA_KIND_137_04 *)cre_ctl.RcvData;
//	Recv137_06 = (DATA_KIND_137_06 *)cre_ctl.RcvData;
//	Recv137_08 = (DATA_KIND_137_08 *)cre_ctl.RcvData;
//	Recv137_0A = (DATA_KIND_137_0A *)cre_ctl.RcvData;
//
//	switch( Recv137_02->Common.DataIdCode1 ){	// 02 ���\�Ŏg��
//	case CRE_RCVCMD_OPEN:
//		result = (ulong)Recv137_02->Result1 * 10000 + (ulong)Recv137_02->Result2;
//		break;
//	case CRE_RCVCMD_CONFIRM:
//		result = (ulong)Recv137_04->Result1 * 10000 + (ulong)Recv137_04->Result2;
//		break;
//	case CRE_RCVCMD_SALES:
//		result = (ulong)Recv137_06->Result1 * 10000 + (ulong)Recv137_06->Result2;
//		break;
//	case CRE_RCVCMD_ONLINETEST:
//		result = (ulong)Recv137_08->Result1 * 10000 + (ulong)Recv137_08->Result2;
//		break;
//	case CRE_RCVCMD_RETURN:
//		result = (ulong)Recv137_0A->Result1 * 10000 + (ulong)Recv137_0A->Result2;
//		break;
//	default:
//		// �z��O�̎�ʂ͖���
//		return retsts;
//	}
//
//	// �t�F�[�Y���� & �ǂ��ԃ`�F�b�N
//	retsts = creResultCheck();
//	if( retsts != 0 ){
//		return retsts;	// �G���[����̏ꍇ�͓ǂݎ̂Ă�B�������ʇ@�A�A���G���[�̏ꍇ�͒ʂ��B
//	}
//
//	// �R�}���h������M���j�^�o�^
//	creRegMonitor(OPMON_CRE_RECV_RESPONSE, (uchar)Recv137_02->Common.DataIdCode1, result);
//
//	//                  //
//	// ��M��ʂ��Ə��� //
//	//                  //
//	switch( Recv137_02->Common.DataIdCode1 ){	// 02 ���\�Ŏg��
//	case CRE_RCVCMD_OPEN:		// �J�ǃR�}���h����(02)
//		if( cre_ctl.Status == CRE_STS_WAIT_OPEN ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_02->Result1;
//			if( retsts == 0 ){
//				cre_ctl.Initial_Connect_Done = 1;	// �J�Ǌ���
//				err_chk2( ERRMDL_CREDIT, ERR_CREDIT_OPEN_ERR, 0x00, 0, 0, NULL );	// �G���[(����)
//				//����˗��t���O ON ?�i����˗�����(06)����M�j
//				if( cre_uriage.UmuFlag != 0 ){
//					ReSendFlg = 1;		// ����˗�(05)�̍đ��M
//					kind = CRE_KIND_AUTO;				// ���M�v���Ɏ�����ݒ�
//				} else {
//					// �O���Ɖ�t���OON�i�^�M�⍇�����M����ׂɁA�J�ǃR�}���h�𑗐M�����j
//					if( OPECTL.InquiryFlg == 1 ){
//						ReSendFlg = 2;		// �^�M�⍇��(03)�̍đ��M
//					}
//				}
//			}else{
//				// �P��ڂ̊J�ǃ��g���C���^�C���A�E�g�����Ƃ��ɃG���[�o�^����
//				if( cre_ctl.Open.Cnt == 1 ){
//					if( cre_ctl.Initial_Connect_Done == 0 ) {	// ���J�ǂ̏ꍇ�̂݃G���[�Ɠo�^����
//						err_chk2( ERRMDL_CREDIT, ERR_CREDIT_OPEN_ERR, 0x01, 0, 0, NULL );	// �G���[(����)
//					}
//				}
//				// �J�ǃR�}���h���M���ɕύX�i���g���C���Ԃ̌v���J�n�j
//				if( cre_ctl.Initial_Connect_Done == 0 ) {	// ���J�ǂ̏ꍇ�̂݃��g���C����
//					creOpen_Enable();
//				}
//			}
//			queset( OPETCBNO, ( retsts==0 ? CRE_EVT_02_OK : CRE_EVT_02_NG ), 0, 0 );	// OpeMain�֒ʒm
//		}
//		break;
//	case CRE_RCVCMD_CONFIRM:	// �^�M�⍇�����ʃf�[�^(04)
//		if( cre_ctl.Status == CRE_STS_WAIT_CONFIRM ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_04->Result1;
//			if( retsts == 0 ){
//				// �W�v�G���A�Ƃ��̑� �X�V
//				creUpdatePayData( Recv137_04 );
//				queset( OPETCBNO, CRE_EVT_04_OK, 0, 0 );	// OpeMain�֒ʒm
//			}else{
//				if(
//					( retsts == 3 ) ||
//					( retsts >= 10 && retsts <= 19 ) ||
//					( retsts >= 20 && retsts <= 29 ) ||
//					( retsts >= 30 && retsts <= 39 ) ||
//					( retsts >= 40 && retsts <= 49 )
//				){
//					queset( OPETCBNO, CRE_EVT_04_NG1, 0, 0 );	// OpeMain�֒ʒm
//				}else{
//					queset( OPETCBNO, CRE_EVT_04_NG2, 0, 0 );	// OpeMain�֒ʒm
//				}
//			}
//		}
//		break;
//	case CRE_RCVCMD_SALES:		// ����˗����ʃf�[�^(06)
//		if( cre_ctl.Status == CRE_STS_WAIT_SALES ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_06->Result1;
//			if( retsts == 0 ){
//				creSales_Reset();	// ����˗��f�[�^�̌o�ߎ��ԃJ�E���g�����Z�b�g
//				cre_uriage.UmuFlag = OFF;						// ����˗��f�[�^(05)���M���׸�ON�i06��M��OFF�j
//				memset( &cre_uriage, 0, sizeof(cre_uriage));	// ����˗��f�[�^�G���A���N���A
//				if( creErrorCheck() == 0 ){
//					cre_ctl.Credit_Stop = 0;	// �N���W�b�g��~�v�����Ȃ��Ȃ������������
//				}
//			}else{
//				// �m�f��������M�����ꍇ�́A���㋑�ۃf�[�^�ɓo�^����
//				if( Recv137_06->Result1 == 99 ) {				// �������ʇ@=99�̏ꍇ�̓^�C���A�E�g�Ɠ��l�ɔ��㋑�ۂƂ��Ȃ�
//					// �������ʇ@=99�̏ꍇ�̓^�C���A�E�g�Ɠ��l�̏������s��
//					creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_SALES, 0);	// ���j�^�o�^
//					// �O���Ɖ�t���OON�̂Ƃ�
//					if( OPECTL.InquiryFlg == 1 ){
//						queset( OPETCBNO, CRE_EVT_06_TOUT, 0, 0 );	// OpeMain�֒ʒm�i�⍇�����̏�Ԃ���������ׁj
//						tout_flg = 1;				// �^�C���A�E�g�ʒm����^�Ȃ�
//					}
//				}else{
//					creSaleNG_Add();							// �N���W�b�g���㋑�ۃf�[�^�o�^
//					err_chk2( ERRMDL_CREDIT, ERR_CREDIT_NO_ACCEPT, 0x01, 0, 0, NULL );		// �G���[(����)
//					creSales_Reset();	// ����˗��f�[�^�̌o�ߎ��ԃJ�E���g�����Z�b�g
//					cre_uriage.UmuFlag = OFF;						// ����˗��f�[�^(05)���M���׸�ON�i06��M��OFF�j
//					memset( &cre_uriage, 0, sizeof(cre_uriage));	// ����˗��f�[�^�G���A���N���A
//					switch( creErrorCheck() ){
//					case 0:		// �G���[�Ȃ�
//						cre_ctl.Credit_Stop = 0;	// �N���W�b�g��~�v�����Ȃ��Ȃ������������
//						break;
//					case 1:		// ����˗������M
//						;	// �Y�������Ȃ�
//						break;
//					case 2:		// ���㋑�ۃf�[�^�l�`�w
//					default:	// ����
//						// �O���Ɖ�t���OON�̂Ƃ�
//						if( OPECTL.InquiryFlg == 1 ){
//							queset( OPETCBNO, CRE_EVT_06_TOUT, 0, 0 );	// OpeMain�֒ʒm�i�⍇�����̏�Ԃ���������ׁj
//							tout_flg = 1;			// �^�C���A�E�g�ʒm����^�Ȃ�
//						}
//						break;
//					}
//				}
//			}
//			if( cre_ctl.Credit_Stop == 0 ) {	// �N���W�b�g��~���łȂ��Ƃ��̂ݗ^�M�⍇���𑗐M����
//				if( Recv137_06->Result1 != 99 ) {	// �������ʇ@=99�ȊO�̏ꍇ�̂ݗ^�M�⍇���𑗐M����
//					// �O���Ɖ�t���OON�i�^�M�⍇�����M����ׂɁA�J�ǃR�}���h�𑗐M�����j
//					if( OPECTL.InquiryFlg == 1 ){
//						ReSendFlg = 2;		// �^�M�⍇��(03)�̍đ��M
//					}
//				}
//			}
//			if( tout_flg == 0 ){				// �^�C���A�E�g�ʒm�Ȃ�
//				queset( OPETCBNO, ( retsts==0 ? CRE_EVT_06_OK : CRE_EVT_06_NG ), 0, 0 );	// OpeMain�֒ʒm
//			}
//		}
//		break;
//	case CRE_RCVCMD_ONLINETEST:	// ��ײ�ýĉ���(08)
//		if( cre_ctl.Status == CRE_STS_WAIT_ONLINETEST ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_08->Result1;
//			if(retsts == 0){
//				// �O���Ɖ�t���OON�i�����i�ɂ�鐶���m�F���ɃN���W�b�g�J�[�h�}�����ꂽ�ꍇ�j
//				if( OPECTL.InquiryFlg == 1 ){
//					ReSendFlg = 2;		// �^�M�⍇��(03)�̑��M
//				}
//			}else{
//				//// �T�[�o�[�Œv���I�ȃG���[���������Ă���G���[�R�[�h���Z�b�g����Ă����ꍇ�́A�����ŃN���W�b�g��~�ɂ���B
//				//cre_ctl.Credit_Stop = 1;	// �N���W�b�g��~
//			}
//			queset( OPETCBNO, ( retsts==0 ? CRE_EVT_08_OK : CRE_EVT_08_NG ), 0, 0 );	// OpeMain�֒ʒm
//		}
//		break;
//	case CRE_RCVCMD_RETURN:		// �ԕi�⍇�����ʃf�[�^(0A)
//		if( cre_ctl.Status == CRE_STS_WAIT_RETURN ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_0A->Result1;
//			if( retsts == 0 ){
//				;	// �Y�������Ȃ�
//			}else{
//				;	// �Y�������Ȃ�
//			}
//			queset( OPETCBNO, ( retsts==0 ? CRE_EVT_0A_OK : CRE_EVT_0A_NG ), 0, 0 );	// OpeMain�֒ʒm
//		}
//		break;
//	}
//	if( cre_ctl.Status == CRE_STS_IDLE ){
//		Lagcan( OPETCBNO, 9 );
//	}
//	switch( ReSendFlg ){
//	case 1:
//		creSales_Send(kind);	// ����˗��f�[�^�đ��M(05)
//		break;
//	case 2:
//		creSendData_CONFIRM();	// �^�M�⍇���f�[�^���M(03)
//		break;
//	}
//
//	return retsts;
//}
//
///*[]-----------------------------------------------------------------------[]*/
///*|  �N���W�b�g���㋑�ۃf�[�^  �f�[�^�o�^                                   |*/
///*[]-----------------------------------------------------------------------[]*/
///*| MODULE NAME  : CRE_SaleNG_Delete                                        |*/
///*| PARAMETER    : ofs	= �폜�������f�[�^�̃e�[�u���擪����̃I�t�Z�b�g    |*/
///*| RETURN VALUE : none                                                     |*/
///*[]-----------------------------------------------------------------------[]*/
///*| REMARK       : �폜��̐擪�f�[�^���z���[0]�ɂ���悤�f�[�^���\��������|*/
///*|                                                                         |*/
///*[]-----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                               |*/
///*| Update       :                                                          |*/
///*[]-------------------------------------  Copyright(C) 2013 AMANO Corp.---[]*/
//void	creSaleNG_Add( void )
//{
//	short i;
//
//	if( cre_saleng.ken >= CRE_SALENG_MAX ){
//		return;	// �ő匏���ɒB���Ă���
//	}
//
//	memset( &cre_saleng_work, 0, sizeof(CRE_SALENG) );
//	memcpy( &cre_saleng_work.back[0], &cre_saleng.back[0], sizeof(struct DATA_BK) * cre_saleng.ken );
//
//	i = (short)cre_saleng.ken;
//	cre_saleng_work.ken = (char)(i + 1);
//
//	cre_saleng_work.back[i].ryo = cre_uriage.back.ryo;														// �ޯ����ߗ���
//	memcpy( &cre_saleng_work.back[i].time, &cre_uriage.back.time, sizeof(cre_saleng_work.back[0].time) );	// ���Z����
//	cre_saleng_work.back[i].slip_no = cre_uriage.back.slip_no;												// �ڼޯĶ��ޓ`�[�ԍ�
//	memcpy( &cre_saleng_work.back[i].AppNoChar[0], &cre_uriage.back.AppNoChar[0], 6 );						// �ڼޯĶ��ޏ��F�ԍ�(�p����)
//	memcpy( &cre_saleng_work.back[i].shop_account_no[0], &cre_uriage.back.shop_account_no[0], 20 );			// �ڼޯĶ��މ����X����ԍ�
//
//	nmisave( &cre_saleng, &cre_saleng_work, sizeof(CRE_SALENG) );
//
//	memset( &cre_uriage, 0, sizeof( cre_uriage ) );		// �N���W�b�g����˗��f�[�^
//
//	if( cre_saleng.ken >= CRE_SALENG_MAX ){
//		// �ꌏ�ǉ���ɍő匏���ɒB�����ꍇ
//		cre_ctl.Credit_Stop	= 1;		// �ڼޯĒ�~
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| �̎��� (�W�v)�p�G���A�X�V												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creUpdatePayData 										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creUpdatePayData( DATA_KIND_137_04 *RcvData )
//{
//	ushort	num;
//	int		i;
//	ulong	turisen_wk = 0;
//	//�W�v�E�̎��� �p
//	memset( &ryo_buf.credit, 0, sizeof(credit_use) );
//
//	memcpy( &ryo_buf.credit.card_name[0], &RcvData->CompanyName[0], 12 );					// �ڼޯĶ��މ�З���
//	memcpy( &ryo_buf.credit.card_no[0], &RcvData->CardNo[0], 16 );
//	AesCBCDecrypt((uchar*)&ryo_buf.credit.card_no[0], 16);
//																							// �ڼޯĶ��މ���ԍ��i�E�l�߁j
//	ryo_buf.credit.slip_no				= RcvData->SlipNo;									// �ڼޯĶ��ޓ`�[�ԍ�
//	ryo_buf.credit.pay_ryo				= creSeisanInfo.amount;								// �N���W�b�g���ϊz
//	ryo_buf.credit.CenterProcTime.Year	= RcvData->Pay.Year;								// �ڼޯĶ��޾������������i�b����j
//	ryo_buf.credit.CenterProcTime.Mon	= RcvData->Pay.Mon;									// �V
//	ryo_buf.credit.CenterProcTime.Day	= RcvData->Pay.Day;									// �V
//	ryo_buf.credit.CenterProcTime.Hour	= RcvData->Pay.Hour;								// �V
//	ryo_buf.credit.CenterProcTime.Min	= RcvData->Pay.Min;									// �V
//	ryo_buf.credit.CenterProcTime.Sec	= RcvData->Pay.Sec;									// �V
//	memcpy( &ryo_buf.credit.ShopAccountNo[0], &RcvData->ShopAccountNo[0], 20 );				// �����X����ԍ�
//	
//	memset( &ryo_buf.credit.AppNoChar[0], 0x20, sizeof(ryo_buf.credit.AppNoChar) );
//	memcpy( &ryo_buf.credit.AppNoChar[0], &RcvData->AppNo[0], sizeof(RcvData->AppNo) );		// �[�����ʔԍ�
//	ryo_buf.credit.app_no = creConvAtoB( &RcvData->AppNo[0], 6 );							// �ڼޯĶ��ޏ��F�ԍ�(�ʐM�p)
//	
//	memset( &ryo_buf.credit.CCT_Num[0], 0x20, sizeof(ryo_buf.credit.CCT_Num) );
//	memcpy( &ryo_buf.credit.CCT_Num[0], &RcvData->TerminalId[0], sizeof(RcvData->TerminalId) );	// �[�����ʔԍ�
//	
//	memcpy( &ryo_buf.credit.kid_code[0], &RcvData->KidCode[0], 6 );							// KID �R�[�h
//
//	//���Z���f�[�^ �p
//	NTNET_Data152Save((void *)(&ryo_buf.credit.pay_ryo), NTNET_152_CPAYRYO);				// �N���W�b�g���ϊz
//	NTNET_Data152Save((void *)(&ryo_buf.credit.card_no[0]), NTNET_152_CCARDNO);				// �N���W�b�g�J�[�h����ԍ�
//	NTNET_Data152Save((void *)(&ryo_buf.credit.CCT_Num[0]), NTNET_152_CCCTNUM);				// �[�����ʔԍ�
//	NTNET_Data152Save((void *)(&ryo_buf.credit.kid_code[0]), NTNET_152_CKID);				// �j�h�c�R�[�h
//	NTNET_Data152Save((void *)(&ryo_buf.credit.app_no), NTNET_152_CAPPNO);
//// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
//	NTNET_Data152Save((void *)(&ryo_buf.credit.ShopAccountNo[0]), NTNET_152_CTRADENO);
//	NTNET_Data152Save((void *)(&ryo_buf.credit.slip_no), NTNET_152_SLIPNO);
//// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
//
//	// ������
//	cre_uriage.back.ryo			= creSeisanInfo.amount;					// �ڼޯĔ�����z�icreSeisanInfo.amount �� ryo_buf.credit.pay_ryo �͓����l�j
//	cre_uriage.back.time.Year	= RcvData->Pay.Year;					// ���Z �N
//	cre_uriage.back.time.Mon	= RcvData->Pay.Mon;						//      ��
//	cre_uriage.back.time.Day	= RcvData->Pay.Day;						//      ��
//	cre_uriage.back.time.Hour	= RcvData->Pay.Hour;					//      ��
//	cre_uriage.back.time.Min	= RcvData->Pay.Min;						//      ��
//	cre_uriage.back.slip_no		= RcvData->SlipNo;						// �[�������ʔԁi�`�[���j
//	memcpy( &cre_uriage.back.AppNoChar[0], &RcvData->AppNo[0], 6 );		// �ڼޯĶ��ޏ��F�ԍ�
//	memcpy( &cre_uriage.back.shop_account_no[0],
//		&RcvData->ShopAccountNo[0], CRE_SHOP_ACCOUNTBAN_MAX );			// �ڼޯĶ��މ����X����ԍ�
//	cre_uriage.back.CMachineNo = 0;										// ���Ԍ��@�B��(���ɋ@�B��)	0�`255
//	cre_uriage.back.PayMethod2 = (uchar)OpeNtnetAddedInfo.PayMethod;	// ���Z���@(0�����Ȃ����Z/1�����Ԍ����Z/2����������Z/3����������p���Z/4���������Z)
//	cre_uriage.back.PayClass = (uchar)OpeNtnetAddedInfo.PayClass;		// �����敪(0�����Z/1���Đ��Z/2�����Z���~
//
//	for( i = 0; i < WTIK_USEMAX; i++ ){
//		if (PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU) break;
//		if (PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE) break;
//// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
////// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
////		if (PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE_2) break;
////// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
//// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//	}
//	if( i != WTIK_USEMAX ){
//		// ���z/�U�֐��Z����
//		cre_uriage.back.PayMode = 4;										// ���Z���[�h(���u���Z)
//	}else{
//		cre_uriage.back.PayMode = 0;										// ���Z���[�h(�������Z)
//	}
//// �d�l�ύX(S) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//	for( i = 0; i < DETAIL_SYU_MAX; i++ ){
//		if (PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2) break;
//	}
//	if( i != DETAIL_SYU_MAX ){
//		cre_uriage.back.PayMode = 4;										// ���Z���[�h(���u���Z)
//	}
//// �d�l�ύX(E) K.Onodera 2016/11/01 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//
//	if( ryo_buf.pkiti == 0xffff ){// �}���`���Z?
//		cre_uriage.back.FlapArea = (ushort)(LOCKMULTI.LockNo / 10000L);		// ���(1�`26)
//		cre_uriage.back.FlapParkNo = (ushort)(LOCKMULTI.LockNo % 10000L);	// ���Ԉʒu��(1�`9999)
//	}else{
//		num = ryo_buf.pkiti - 1;
//		cre_uriage.back.FlapArea = (ushort)LockInfo[num].area;				// ���(1�`26)
//		cre_uriage.back.FlapParkNo = (ushort)LockInfo[num].posi;			// ���Ԉʒu��(1�`9999)
//	}
//	if( ryo_buf.ryo_flg < 2 ){
//		cre_uriage.back.Price = ryo_buf.tyu_ryo;							// ���ԗ���
//	}else{// ������Z����
//		cre_uriage.back.Price = ryo_buf.tei_ryo;							// �������
//	}
//	cre_uriage.back.OptionPrice = 0;										// ���̑�����	0�`
//	if(	ryo_buf.dsp_ryo < ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo )){
//		// �����ƁA�N���W�b�g�F���̂���Ⴂ��ryo_buf.nyukin�ɂ͉��Z����邪���̎��_��ryo_buf.turisen�ɉ��Z����Ȃ��̂ł����Ŕ��f����
//		// �ȉ��ŗ]�v�ɓ������ꂽ�����Z�o����
//		turisen_wk = ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo ) - ryo_buf.dsp_ryo;
//		cre_uriage.back.CashPrice = ryo_buf.nyukin - turisen_wk;// ��������(�����̎����z)
//	}else{
//		cre_uriage.back.CashPrice = ryo_buf.nyukin;			// ��������(�����̎����z)
//	}
//	// �����z = ���ԗ��� - ��������z - �N���W�b�g������z
//	cre_uriage.back.Discount = cre_uriage.back.Price - cre_uriage.back.CashPrice - ryo_buf.credit.pay_ryo;// �����z(���Z)
//	CountGet( PAYMENT_COUNT, &PayData.Oiban );								// �ǔԎ擾
//	CountFree(PAYMENT_COUNT);// �����ň�U�ԋp�APayData_set()�֐��ł��擾���邽��
//	cre_uriage.back.PayCount = CountSel( &PayData.Oiban );// ���Z�ǔ�(0�`99999)
//
//	// FT-4000�ł́u���Z�}�́v�ɐݒ�ł���J�[�h�͒�����݂̂Ȃ̂Œ���̎g�p�L���Ŕ��f����
//	memset( &cre_uriage.back.Media.MediaCardNo[0], 0, sizeof( cre_uriage.back.Media.MediaCardNo ) );
//	memset( &cre_uriage.back.Media.MediaCardInfo[0], 0, sizeof( cre_uriage.back.Media.MediaCardInfo ) );
//	if( ryo_buf.ryo_flg >= 2 ){
//		cre_uriage.back.MediaKind = PayData.teiki.pkno_syu+1;// ��������ԏꇂ
//		intoasl(&cre_uriage.back.Media.MediaCardNo[0], PayData.teiki.id, 5);		// �J�[�h�ԍ�	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//		intoasl(&cre_uriage.back.Media.MediaCardInfo[0], PayData.teiki.syu, 2);		// �J�[�h���	"0"�`"9"�A"A"�`"Z"(�p����) ���l��
//	}else{
//		cre_uriage.back.MediaKind = 0;// ��ʂȂ�
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| TimeOut ���� (OPETCBNO,Timer = 12)										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creTimeOut												|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////|					-1:���g���C���M���s										|
////|					-2:CRE�d�������^�C���A�E�g								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creTimeOut( void )
//{
//	short 	retsts = 0;
//
//	cre_ctl.Timeout_Announce = 0;	// ��M�^�C���A�E�g���̃A�i�E���X�ۃt���O
//
//
//	switch( cre_ctl.Status ){
//	case CRE_STS_WAIT_OPEN:			// 02�҂�
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_OPEN, 0);	// ���j�^�o�^
//		retsts = -2;
//		// �P��ڂ̊J�ǃ��g���C���^�C���A�E�g�����Ƃ��ɃG���[�o�^����
//		if( cre_ctl.Open.Cnt == 1 ){
//			if( cre_ctl.Initial_Connect_Done == 0 ) {	// ���J�ǂ̏ꍇ�̂݃G���[�Ɠo�^����
//				err_chk2( ERRMDL_CREDIT, ERR_CREDIT_OPEN_ERR, 0x01, 0, 0, NULL );	// �G���[(����)
//			}
//		}
//		// �O���Ɖ�t���OON�i�^�M�⍇�����M����ׂɁA�J�ǃR�}���h�𑗐M�����j
//		queset( OPETCBNO, CRE_EVT_02_NG, 0, 0 );	// OpeMain�֒ʒm�i�⍇�����̏�Ԃ���������ׁj
//		if( cre_ctl.Initial_Connect_Done == 0 ) {	// ���J�ǂ̏ꍇ�̂݃��g���C����
//			creOpen_Enable();
//		}
//		break;
//	case CRE_STS_WAIT_CONFIRM:		// 04�҂�
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_CONFIRM, 0);	// ���j�^�o�^
//		retsts = -2;
//		cre_ctl.Timeout_Announce = 1;	// ��M�^�C���A�E�g���̃A�i�E���X�ۃt���O
//		break;
//	case CRE_STS_WAIT_SALES:		// 06�҂�
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_SALES, 0);	// ���j�^�o�^
//		retsts = -2;
//		// �O���Ɖ�t���OON�̂Ƃ�
//		if( OPECTL.InquiryFlg == 1 ){
//			queset( OPETCBNO, CRE_EVT_06_TOUT, 0, 0 );	// OpeMain�֒ʒm�i�⍇�����̏�Ԃ���������ׁj
//		}
//		break;
//	case CRE_STS_WAIT_ONLINETEST:	// 08�҂�
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_ONLINETEST, 0);	// ���j�^�o�^
//		retsts = -2;
//		err_chk2( ERRMDL_CREDIT, ERR_CREDIT_CONNECTCHECK_ERR, 0x01, 0, 0, NULL );	// �G���[(����)
//		// �O���Ɖ�t���OON�̂Ƃ�
//		if( OPECTL.InquiryFlg == 1 ){
//			queset( OPETCBNO, CRE_EVT_08_NG, 0, 0 );	// OpeMain�֒ʒm�i�⍇�����̏�Ԃ���������ׁj
//		}
//		break;
//	case CRE_STS_WAIT_RETURN:		// 0A�҂�
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_RETURN, 0);	// ���j�^�o�^
//		retsts = -2;
//		break;
//	}
//	if( retsts != 0 ){	// ��ײ�ȊO
//		cre_ctl.Status = CRE_STS_IDLE;
//	}
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| CRE �d���߂���`�F�b�N&�G���[����										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creResultCheck											|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : 0 : ����, -1: �t�F�[�Y����, -2: �������G���[				|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creResultCheck( void )
//{
//	short	retsts = 0;
//	DATA_KIND_137_02	*Recv137_02;
//	DATA_KIND_137_04	*Recv137_04;
//	DATA_KIND_137_06	*Recv137_06;
//	DATA_KIND_137_08	*Recv137_08;
//	DATA_KIND_137_0A	*Recv137_0A;
//
//	Recv137_02 = (DATA_KIND_137_02 *)cre_ctl.RcvData;
//	Recv137_04 = (DATA_KIND_137_04 *)cre_ctl.RcvData;
//	Recv137_06 = (DATA_KIND_137_06 *)cre_ctl.RcvData;
//	Recv137_08 = (DATA_KIND_137_08 *)cre_ctl.RcvData;
//	Recv137_0A = (DATA_KIND_137_0A *)cre_ctl.RcvData;
//
//	cre_ctl.Timeout_Announce = 0;	// ��M�^�C���A�E�g���̃A�i�E���X�ۃt���O
//
//	switch( cre_ctl.Status ){
//	case CRE_STS_WAIT_OPEN:			// 02�҂�
//		if( Recv137_02->Common.DataIdCode1 == CRE_RCVCMD_OPEN ){
//			// �����ǂ��ԃ`�F�b�N
//			if( Recv137_02->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// �J�ǔN�����`�F�b�N
//			if(
//				(Recv137_02->Proc.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_02->Proc.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_02->Proc.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_02->Proc.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_02->Proc.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_02->Proc.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		break;
//	case CRE_STS_WAIT_CONFIRM:		// 04�҂�
//		if( Recv137_04->Common.DataIdCode1 == CRE_RCVCMD_CONFIRM ){
//			// �����ǂ��ԃ`�F�b�N
//			if( Recv137_04->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// ���Z�N�����`�F�b�N
//			if(
//				(Recv137_04->Pay.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_04->Pay.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_04->Pay.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_04->Pay.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_04->Pay.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_04->Pay.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//			// �[�������ʔԁi�`�[�ԍ��j�`�F�b�N
//			if( Recv137_04->SlipNo != cre_ctl.Save.SlipNo ){
//				retsts = -2;
//				break;
//			}
//			// ������z�`�F�b�N
//			if( Recv137_04->Amount != cre_ctl.Save.Amount ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		cre_ctl.Timeout_Announce = 1;	// ��M�^�C���A�E�g���̃A�i�E���X�ۃt���O
//		break;
//	case CRE_STS_WAIT_SALES:		// 06�҂�
//		if( Recv137_06->Common.DataIdCode1 == CRE_RCVCMD_SALES ){
//			// �����ǂ��ԃ`�F�b�N
//			if( Recv137_06->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// ���Z�N�����`�F�b�N
//			if(
//				(Recv137_06->Pay.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_06->Pay.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_06->Pay.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_06->Pay.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_06->Pay.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_06->Pay.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//			// �[�������ʔԁi�`�[�ԍ��j�`�F�b�N
//			if( Recv137_06->SlipNo != cre_ctl.Save.SlipNo ){
//				retsts = -2;
//				break;
//			}
//			// ������z�`�F�b�N
//			if( Recv137_06->Amount != cre_ctl.Save.Amount ){
//				retsts = -2;
//				break;
//			}
//			// ���F�ԍ��`�F�b�N
//			if( 0 != memcmp( &Recv137_06->AppNo[0], &cre_ctl.Save.AppNo[0], 6 ) ){
//				retsts = -2;
//				break;
//			}
//			// �����X����ԍ��`�F�b�N
//			if( 0 != memcmp( &Recv137_06->ShopAccountNo[0], &cre_ctl.Save.ShopAccountNo[0], 20 ) ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		cre_ctl.Timeout_Announce = 1;	// ��M�^�C���A�E�g���̃A�i�E���X�ۃt���O
//		break;
//    case CRE_STS_WAIT_ONLINETEST:	// 08�҂�
//		if( Recv137_08->Common.DataIdCode1 == CRE_RCVCMD_ONLINETEST ){
//			// �����ǂ��ԃ`�F�b�N
//			if( Recv137_08->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// �e�X�g���
//			if( Recv137_08->TestKind != cre_ctl.Save.TestKind ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		break;
//	case CRE_STS_WAIT_RETURN:		// 0A�҂�
//		if( Recv137_0A->Common.DataIdCode1 == CRE_RCVCMD_RETURN ){
//			// �����ǂ��ԃ`�F�b�N
//			if( Recv137_0A->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// �����N�����`�F�b�N
//			if(
//				(Recv137_0A->Proc.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_0A->Proc.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_0A->Proc.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_0A->Proc.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_0A->Proc.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_0A->Proc.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//			// �[�������ʔԁi�`�[�ԍ��j�`�F�b�N
//			if( Recv137_0A->SlipNo != cre_ctl.Save.SlipNo ){
//				retsts = -2;
//				break;
//			}
//			// ���F�ԍ��`�F�b�N
//			if( 0 != memcmp( &Recv137_0A->AppNo[0], &cre_ctl.Save.AppNo[0], 6 ) ){
//				retsts = -2;
//				break;
//			}
//			// �����X����ԍ��`�F�b�N
//			if( 0 != memcmp( &Recv137_0A->ShopAccountNo[0], &cre_ctl.Save.ShopAccountNo[0], 20 ) ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		break;
//	}
//
//	return retsts;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*| �E�񂹂ɂ��ăR�s�[���A�擪���X�y�[�X�Ŗ��߂�                           |*/
///*| ���P �X�y�[�X(0x20)�����łȂ��A�k��(0x0)���폜�ΏۂƂ���               |*/
///*[]----------------------------------------------------------------------[]*/
///*| PARAMETER    : *dist    : �i�[��|�C���^�[                             |*/
///*|				   *src     : �i�[���|�C���^�[                             |*/
///*|				   dist_len : �i�[��o�b�t�@��                             |*/
///*|				   src_len  : �i�[���o�b�t�@��                             |*/
///*| RETURN VALUE : none                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       :                                                         |*/
///*| Date         :                                                         |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//void creMemCpyRight(uchar *dist, uchar *src, ushort dist_len, ushort src_len)
//{
//	ushort	moji_left, moji_len, len, i;
//
//	moji_left	= 0;
//	moji_len	= 0;
//	memset( dist, 0x20, (size_t)dist_len );				// �i�[����X�y�[�X�ŏ�����
//
//	len = src_len;
//	if ( dist_len < src_len ) {
//		len = dist_len;									// �o�b�t�@���̒Z�����ɍ��킹��
//	}
//
//	if ( len < 2 ) return;								// �����񒷂�2��菬����
//
//	// ������̐擪�ʒu����
//	for ( i = 0; i < len; i++ ) {
//		if (( src[i] != 0x0 ) && ( src[i] != 0x20 )) {	// 0x0��0x20�͏Ȃ�
//			break;
//		}
//	}
//	if (i >= len) return;								// �����񂪑��݂��Ȃ�
//	moji_left = i;										// ������擪�ʒu
//
//	// ������̖����ʒu����
//	for ( i = len - 1; i > moji_left; i-- ) {
//		if (( src[i] != 0x0 ) && ( src[i] != 0x20 )) {	// 0x0��0x20�͏Ȃ�
//			break;
//		}
//	}
//	moji_len = (uchar)(( i + 1 ) - moji_left );				// ������
//
//	memcpy( &dist[dist_len - moji_len], &src[moji_left], (size_t)moji_len );	// ��������E�񂹂ŃR�s�[
//}
//
////[]-----------------------------------------------------------------------[]
////| LCD�\���ƃG���[���̃A�i�E���X���s��										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creMessageAnaOnOff 	 									|
////| PARAMETER    : OnOff(i) : 1:�\������ 0:�\����߂�						|
////|				   num(i)   : cre �����\������ݔԍ�							|
////|								2 �ȏ�̓G���[								|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creMessageAnaOnOff( short OnOff, short num )
//{
//	// �O��̌㏈��
//	switch( cre_ctl.MessagePtnNum ){
//	case 1:
//		blink_end();
//	case 4:
//	case 6:
//		Vram_BkupRestr( 0, 6, 1 );	//Line=6 Restore
//		Vram_BkupRestr( 0, 7, 1 );	//Line=7 Restore
//		break;
//	case 2:
//	case 3:
//	case 7:
//	case 8:
//		Vram_BkupRestr( 0, 7, 1 );	//Line=7 Restore
//		break;
//	case 5:
//	case 9:
//		break;
//	}
//	cre_ctl.MessagePtnNum = 0;
//
//	// ����̏���
//	if( OnOff == 1 ){
//		// ��ʕ\��
//		switch( num ){
//		case 1:											// 
//			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[0] );	// Line=6:���]�\�� "�N���W�b�g�J�[�h�Ɖ�ł�"
//															// Line=7:�_�ŕ\�� "���΂炭���҂�������"
//			blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR[7] );			// 
//			break;
//		case 2:											// 
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ERR_CHR[44] );		// Line=7:���]�\�� " �����A����舵�����ł��܂��� "
//			break;
//		case 3:											// "���̃J�[�h�͂��戵���ł��܂���"
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup 	   
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ERR_CHR[1] );		// Line=7:���]�\�� "���̃J�[�h�͎g���܂���"
//			break;
//		case 4:											// 
//			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[1] );	// Line=6:���]�\�� "�N���W�b�g�J�[�h�ł�"
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[2] );	// Line=7:���]�\�� "���Z���x�z�������Ă��܂�"
//			break;
//		case 5:											// 
//			grachr( 3, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[3] );	// Line=3:���] "�N���W�b�g�J�[�h�����A      "
//			grachr( 4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[4] );	// Line=4:���] "�ēx����������Ă�������    "
//			grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[10] );	// Line=6: "���~����ꍇ�́A�y����z�{�^��",
//			grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[11] );	// Line=7: "�������ĉ�����                ",
//			break;
//		case 6:											// 
//			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[5] );	// Line=6:���]�\�� "    ���̃N���W�b�g�J�[�h��    "
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[6] );	// Line=7:���]�\�� "          �g���܂���          "
//			break;
//		case 7:											// 
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[7] );	// Line=7:���]�\�� "�N���W�b�g�J�[�h�����ĉ�����"
//			break;
//		case 8:											// 
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[8] );	// Line=7:���]�\�� "�ŏ��Ɠ�������������ĉ�����"
//			break;
//		case 9:
//			grachr( 3, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[9] );	// Line=3:���]�\�� "����������A                "
//			grachr( 4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[7] );	// Line=4:���]�\�� "�N���W�b�g�J�[�h�����ĉ�����"
//			grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[10] );	// Line=6: "���~����ꍇ�́A�y����z�{�^��",
//			grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[11] );	// Line=7: "�������ĉ�����                ",
//			break;
//		}
//		// �����ē�
//		switch( num ){
//		case 1:
//			ope_anm( AVM_CREWAIT );		// �ųݽ �u���΂炭���҂��������v
//			break;
//		case 3:
//		case 4:
//			ope_anm( AVM_CARD_ERR4 );	// �ųݽ �u���̃J�[�h�͂���舵���o���܂���B�v
//			break;
//		case 2:
//		case 6:
//			ope_anm( AVM_CREJIT_NG );	// �ųݽ �u�����A�N���W�b�g�J�[�h�͂���舵���o���܂���B�v
//			break;
//		case 7:
//		case 8:
//			break;
//		}
//		cre_ctl.MessagePtnNum = num;	// �\���������Ƃ��̂��߂ɕۑ�
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| �ڼޯēd���p�ǂ��ԏ�����												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSeqNo_Init 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSeqNo_Init( void )
//{
//	cre_ctl.SeqNo	= 0;	// �������i�N���㏉��̂݁j
//}
//
////[]-----------------------------------------------------------------------[]
////| �ڼޯēd���p�ǂ��ԃJ�E���g												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSeqNo_Count 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ushort	creSeqNo_Count( void )
//{
//	if( ++cre_ctl.SeqNo == 0 ){	// �J�E���g(+1)
//		cre_ctl.SeqNo = CRE_SEQ_MIN;
//	}
//	return cre_ctl.SeqNo;
//}
//
////[]-----------------------------------------------------------------------[]
////| �ڼޯēd���p�ǂ��Ԃ̎擾												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSeqNo_Get 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ushort	creSeqNo_Get( void )
//{
//	return cre_ctl.SeqNo;		// ���ݒl��Ԃ�
//}
//
////[]-----------------------------------------------------------------------[]
////| �[�������ʔԁi�`�[�ԍ��j�J�E���g										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSlipNo_Count 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ulong	creSlipNo_Count( void )
//{
//	if( ++cre_slipno > CRE_SLIP_MAX ){	// �J�E���g(+1)
//		cre_slipno = CRE_SLIP_MIN;
//	}
//	return cre_slipno;
//}
//
////[]-----------------------------------------------------------------------[]
////| �[�������ʔԁi�`�[�ԍ��j�̎擾											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSlipNo_Get 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ulong	creSlipNo_Get( void )
//{
//	return cre_slipno;				// ���ݒl��Ԃ�
//}
//
////[]-----------------------------------------------------------------------[]
////| ����˗��f�[�^�̃��g���C����f�[�^������								|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Init		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Init( void )
//{
//	memset( &cre_ctl.Sales, 0, sizeof(cre_ctl.Sales) );	// �N���A
//
//	// ����˗��f�[�^�đ����g���C�Ԋu
//	cre_ctl.Sales.Wait[0] = CRE_RETRY_WAIT_0;	// ��
//	cre_ctl.Sales.Wait[1] = CRE_RETRY_WAIT_1;	// ��
//}
//
////[]-----------------------------------------------------------------------[]
////| ����˗��f�[�^�̌o�ߎ��ԃJ�E���g�����Z�b�g								|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Reset	 			 							|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Reset( void )
//{
//	cre_ctl.Sales.Cnt = 0;
//	cre_ctl.Sales.Stage = 0;
//	cre_ctl.Sales.Min = 0;
//}
//
////[]-----------------------------------------------------------------------[]
////| ����˗��f�[�^�̌o�ߎ��Ԃ��`�F�b�N										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Check		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creSales_Check( void )
//{
//	uchar	ret = 0;
//
//	// �o�ߎ��ԃ`�F�b�N
//	if( ++cre_ctl.Sales.Min >= cre_ctl.Sales.Wait[ cre_ctl.Sales.Stage ] ){
//		cre_ctl.Sales.Min = 0;		// �������Z�b�g
//		ret = 1;
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| ����˗��f�[�^�đ��񐔂��J�E���g										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Count 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Count( void )
//{
//	if( ++cre_ctl.Sales.Cnt == CRE_RETRY_CNT_0 ){
//		cre_ctl.Sales.Stage = 1;		// 0:�Q��ڂ܂ŁA1:�R��ڈȍ~
//	}
//	if( cre_ctl.Sales.Cnt >= CRE_RETRY_CNT_1 ){
//		cre_ctl.Sales.Cnt = CRE_RETRY_CNT_1;	// �R��ȏ�̓J�E���g���Ȃ�
//	}
//	cre_ctl.Sales.Min = 0;				// �������Z�b�g
//}
//
////[]-----------------------------------------------------------------------[]
////| ����˗��f�[�^�đ�����													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Send 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Send( short kind )
//{
//	cre_ctl.SalesKind = kind;
//
//	// ����˗��f�[�^(05)�𑗐M����
//	creSendData_SALES();
//}
//
////[]-----------------------------------------------------------------------[]
////| �J�ǃR�}���h�̃��g���C����f�[�^������									|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Init 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Init( void )
//{
//	memset( &cre_ctl.Open, 0, sizeof(cre_ctl.Open) );	// �N���A
//
//	// �J�ǃR�}���h���g���C�Ԋu
//	cre_ctl.Open.Wait[0] = CRE_RETRY_WAIT_0;	// ��
//	cre_ctl.Open.Wait[1] = CRE_RETRY_WAIT_1;	// ��
//}
//
////[]-----------------------------------------------------------------------[]
////| �J�ǃR�}���h�̃��g���C���M����											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Enable 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Enable( void )
//{
//	cre_ctl.Open.Min = 0;	// ���M��
//}
//
////[]-----------------------------------------------------------------------[]
////| �J�ǃR�}���h�̃��g���C���M�֎~											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Disable		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Disable( void )
//{
//	cre_ctl.Open.Min = (char)-1;	// ���M�s��
//}
//
////[]-----------------------------------------------------------------------[]
////| �J�ǃR�}���h�̌o�ߎ��Ԃ��`�F�b�N										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Check 			 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creOpen_Check( void )
//{
//	uchar	ret = 0;
//
//	if( cre_ctl.Open.Min != (char)-1 ){	// ���M��
//		// �o�ߎ��ԃ`�F�b�N
//		if( ++cre_ctl.Open.Min >= cre_ctl.Open.Wait[ cre_ctl.Open.Stage ] ){
//			cre_ctl.Open.Min = 0;		// �������Z�b�g
//			ret = 1;
//		}
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| �J�ǃR�}���h���M����													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Send 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Send( void )
//{
//	cre_ctl.OpenKind = CRE_KIND_RETRY;	// �J�ǔ����v���Ƀ��g���C��ݒ�
//
//	// �J�ǃR�}���h(01)�𑗐M����
//	if( creSendData_OPEN() == -2 ){
//		return;
//	}
//
//	// �J�ǃR�}���h���M�֎~�ɕύX
//	creOpen_Disable();
//
//	if( ++cre_ctl.Open.Cnt == CRE_RETRY_CNT_0 ){
//		cre_ctl.Open.Stage = 1;		// 0:�Q��ڂ܂ŁA1:�R��ڈȍ~
//	}
//	if( cre_ctl.Open.Cnt >= CRE_RETRY_CNT_1 ){
//		cre_ctl.Open.Cnt = CRE_RETRY_CNT_1;		// �R��ȏ�̓J�E���g���Ȃ�
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ��ײ�ýĂ̐���f�[�^������												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Init 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Init( void )
//{
//	memset( &cre_ctl.OnlineTest, 0, sizeof(cre_ctl.OnlineTest) );	// �N���A
//
//	// ��ײ�ýĂ̎��s�Ԋu
//	cre_ctl.OnlineTest.Wait = (uchar)prm_get( COM_PRM, S_CRE, 65, 2, 1 );	// ��
//	if( cre_ctl.OnlineTest.Wait != 0 &&
//		( cre_ctl.OnlineTest.Wait < 5 || cre_ctl.OnlineTest.Wait > 60) ){
//			/* �ݒ�l���͈͊O�̏ꍇ�͂T�T���Ƃ���(�O�̓I�����C���e�X�g���Ȃ��ݒ�) */
//			cre_ctl.OnlineTest.Wait = 55;
//	}
//
//}
//
////[]-----------------------------------------------------------------------[]
////| ��ײ�ýĂ̌o�ߎ��ԃJ�E���g�����Z�b�g									|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Reset	 		 							|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Reset( void )
//{
//	cre_ctl.OnlineTest.Min = 0;
//}
//
////[]-----------------------------------------------------------------------[]
////| ��ײ�ýĂ̌o�ߎ��Ԃ��J�E���g											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Count 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Count( void )
//{
//	if(cre_ctl.OnlineTest.Wait != 0){
//		++cre_ctl.OnlineTest.Min;
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ��ײ�ýĂ̌o�ߎ��Ԃ��`�F�b�N											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Check 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creOnlineTest_Check( void )
//{
//	uchar	ret = 0;
//
//	if(cre_ctl.OnlineTest.Wait == 0){
//		/* �ݒ�l���O�̏ꍇ�̓I�����C���e�X�g���Ȃ� */
//		return 0;
//	}
//	if( cre_ctl.OnlineTest.Min >= cre_ctl.OnlineTest.Wait ){
//		ret = 1;
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| ��ײ�ýĂ̑��M����														|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Send 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Send( void )
//{
//	// ��ײ�ý�(07)�𑗐M����
//	if( creSendData_ONLINETEST() == -2 ){
//		return;
//	}
//
//	creOnlineTest_Reset();	// ���ʐM���ԃ��Z�b�g
//}
//
////[]-----------------------------------------------------------------------[]
////| �N���W�b�g��񏉊���													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creInfoInit( void )										|
////| PARAMETER    : void														|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creInfoInit( void )
//{
//	memset( &creSeisanInfo, 0, sizeof(creSeisanInfo) );	// �N���W�b�g���Z���N���A
//}
////[]-----------------------------------------------------------------------[]
/////	@brief			�A�X�L�[TO�o�C�i���[�i�P�O�i)
////[]-----------------------------------------------------------------------[]
/////	@param[in]		asc_buf(i)	: ������o�b�t�@�|�C���^�[
/////	@param[in]		len(i)		: ������(MAX=10)
/////	@return			void
////[]-----------------------------------------------------------------------[]
/////	@author			T.Okamoto
/////	@date			Create	: 2006/07/07<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
//static	ulong creConvAtoB( uchar *asc_buf, ushort len )
//{
//	uchar	work[6];
//	ushort	i;
//
//	creMemCpyRight( &work[0], &asc_buf[0], sizeof( work ), len );
//	for( i = 0; i < len; i++ ){
//		if((work[i] < 0x30) || (work[i] > 0x39)){
//			work[i] = 0x30;
//		}
//	}
//	return astoinl( &work[0], (short)len );
//}
//
////[]-----------------------------------------------------------------------[]
////| �N���W�b�g�֘A���j�^�f�[�^�o�^											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creRegMonitor( void )									|
////| PARAMETER    : ushort	code	���j�^��ʃR�[�h						|
////|                uchar	type	�R�}���h���(CRE_SNDCMD_*,CRE_RCVCMD_*) |
////|                ulong	info	�t�����								|
////| RETURN VALUE : void														|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creRegMonitor( ushort code, uchar type, ulong info )
//{
//	uchar	regType;										// ���j�^�o�^�I��
//	uchar	moni_wk[10];									// �o�C�i���f�[�^�o�^�p���[�N
//	
//	regType = (uchar)prm_get( COM_PRM, S_CRE, 67, 1, 1 );
//	if(code == OPMON_CRE_SEND_COMMAND || 					// �R�}���h���M
//		code == OPMON_CRE_RECV_RESPONSE ||					// �R�}���h����
//		code == OPMON_CRE_COMMAND_TIMEOUT) {				// �R�}���h�^�C���A�E�g
//		switch(type) {
//		case CRE_SNDCMD_OPEN:								// �J�ǃR�}���h���M
//		case CRE_RCVCMD_OPEN:								// �J�ǌ��ʎ�M
//			if(regType < 2) {
//				return;										// ���j�^��o�^���Ȃ�
//			}
//			break;
//		case CRE_SNDCMD_CONFIRM:							// �^�M�R�}���h���M
//		case CRE_RCVCMD_CONFIRM:							// �^�M���ʎ�M
//		case CRE_SNDCMD_SALES:								// ����˗��R�}���h���M
//		case CRE_RCVCMD_SALES:								// ����˗����ʎ�M
//			if(regType < 1) {
//				return;										// ���j�^��o�^���Ȃ�
//			}
//			break;
//		default:
//			if(regType < 3) {
//				return;										// ���j�^��o�^���Ȃ�
//			}
//		}
//	}
//	else {
//		return;
//	}
//	
//	switch(code) {
//	case OPMON_CRE_SEND_COMMAND:							// �N���W�b�g�R�}���h���M
//		wmonlg(code, 0, _MAKELONG((ushort)type, (ushort)info));
//		break;
//	case OPMON_CRE_RECV_RESPONSE:							// �N���W�b�g�R�}���h������M
//		memset(moni_wk, 0, sizeof(moni_wk));
//		moni_wk[5] = type;									// �R�}���h���
//		memcpy(&moni_wk[6], &info, sizeof(info));			// ���ʇ@�A
//		wmonlg(code, moni_wk, 0);
//		break;
//	case OPMON_CRE_COMMAND_TIMEOUT:							// �N���W�b�g�R�}���h�^�C���A�E�g
//		wmonlg(code, 0, (ulong)type);
//		break;
//	default:
//		break;
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ���㋑�ۃf�[�^�`�F�b�N													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creCheckRejectSaleData( void )							|
////| PARAMETER    : void														|
////| RETURN VALUE : void														|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creCheckRejectSaleData( void )
//{
//	if( cre_saleng.ken > 0 ) {
//		err_chk2( ERRMDL_CREDIT, ERR_CREDIT_NO_ACCEPT, 0x01, 0, 0, NULL );	// E8861
//	}
//}
short creCtrl(short event)
{
	return 0;
}

void creCtrlInit(uchar flg)
{
}

void creInfoInit(void)
{
}

uchar creStatusCheck(void)
{
	return CRE_STATUS_DISABLED;
}

void creCheckRejectSaleData(void)
{
}

void creOneMinutesCheckProc(void)
{
}

void Credit_main(void)
{
}

uchar Credit_GetSeqFlag(void)
{
	return RAU_SND_SEQ_FLG_NORMAL;
}

void Credit_TcpConnReq_Clear(void)
{
}

void Credit_SetError(uchar ucErrCode)
{
}
// MH810103 GG119202(E) �s�v�@�\�폜(�Z���^�[�N���W�b�g)
