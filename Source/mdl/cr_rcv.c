/************************************************************************************/
/*�@�֐����́@�@�@: ���Cذ�ނ�����ް���M����@�@�@�@		�@�@�@�@�@�@�@�@ �@		*/
/*�@�֐��V���{���@: cr_snd()  �@ �@�@: �@�@�@�@�@�@�@�@�@�@�@	�@�@�@�@�@�@: �@	*/
/*�@�R���p�C���@�@: MS-C ver.6.0�@	: �@�@�@�@�@�@�@�@�@�@�@�@�@	�@�@�@�@: �@	*/
/*�@�^�[�Q�b�gCPU : V25   �@�@�@�@�@: �@�@�@�@�@�@�@�@�@�@�@�@�@�@�@	�@�@: �@	*/
/*�@�쐬�ҁi���t�j: �˓c			  �@�@�@�@�@�@�@�@�@�@�@�@�@�@�@	�@�@: �@	*/
/************************************************************************************/
/* Update:2004.5.13  T.nakayama 													*/
/* 	4500EX :	V25->H8 Arcnet �ڐA	 												*/
/* 		   :	arc_txrx�\���̎Q�ƕ����ύX '->'�� '.'�ɕύX							*/
/* 		   :	TF4800N�\���Ƃ�I/F���Ƃ�											*/
/* 		   :	�����ď����̒ǉ�													*/
/* 		   :	EOT�ð�������̒ǉ� mdltsk.c md_seot()								*/
/************************************************************************************/
#include	<string.h>										/*						*/
#include	"system.h"										/*						*/
#include	"Message.h"										/* Message				*/
#include	"mdl_def.h"										/* Ӽޭ�يԒʐM�@��		*/
#include	"mem_def.h"										/*						*/
#include	"pri_def.h"										/*						*/
#include	"rkn_def.h"										/*						*/
#include	"rkn_cal.h"										/*						*/
#include	"ope_def.h"										/*						*/
#include	"tbl_rkn.h"										/*						*/
#include	"prm_tbl.h"
#include	"fbcom.h"
															/*						*/
															/*						*/
void	cr_rcv( void );										/*						*/
char	cr_queset( char );									/*						*/
void	avm_rcv( void );									/*						*/
char	avm_queset( char);									/*						*/
															/*						*/
static	void 	md_seot(unsigned char *, short );			/*						*/
															/*						*/
char	MTS_rseq[MTS_MAX] = {(char)-1, (char)-1};			/*						*/
uchar	MRD_VERSION[16];									// ���C���[�_�[�o�[�W����
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//extern	char	pcard_shtter_ctl;
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
															/*						*/
void	cr_rcv( void )										/*						*/
{															/*						*/
	char	seq;											/*						*/

	seq = FBcom_RcvData[1];									/* Ӽޭ�يԂ̼��ݼ�ه�	*/
	switch( seq )											/*						*/
	{														/*						*/
	case 0x04:												/* EOT					*/
		cr_queset(seq);										/* ������				*/
		break;												/*						*/
															/*						*/
	case '1':												/*						*/
	case '2':												/*						*/
	case '3':												/*						*/
	case '4':												/*						*/
	case '5':												/*						*/
	case '6':												/*						*/
	case '7':												/*						*/
	case '8':												/*						*/
	case '9':												/*						*/
		if( seq == MTS_rseq[MTS_RED] )						/* ���꼰�ݼ�ه�		*/
		{													/*						*/
			err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 0, 0, 0 );	/* 00:ذ�ޒʐM�ُ����	*/
			return;											/*						*/
		}													/*						*/
															/*						*/
	case '0':												/*						*/
		err_chk( ERRMDL_READER, ERR_RED_COMFAIL, 0, 0, 0 );	/* 00:ذ�ޒʐM�ُ����	*/
		MTS_rseq[MTS_RED] = seq;							/*						*/
															/*						*/
		cr_queset(seq);										/* ������				*/
// MH810103 GG119202(S) IC�N���W�b�g�Ή��iPCI-DSS�j
		memset(FBcom_RcvData, 0, sizeof(FBcom_RcvData));
// MH810103 GG119202(E) IC�N���W�b�g�Ή��iPCI-DSS�j
	}														/*						*/
	return;													/*						*/
}															/*						*/
															/*						*/
/*==================================================================================*/

/*[]------------------------------------------------------------------------------[]*/
/*| Reader ARCNET��M�ް��𷭰�־�Ă���                                            |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Argument : arc_txrx.rxARCdata[1]:���ݼ�ه�	                        		   |*/
/*|          : arc_txrx.rxARCdata[3]:�ް���ۯ���		                   		   |*/
/*|          : arc_txrx.rxARCdata[4]:�ް�����			                   		   |*/
/*|          : arc_txrx.rxARCdata[5]:�ް����			                   		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Return : queset											              		   |*/
/*|        : ARC_CR_R_EVT: ذ�޺���ގ�M R				                   		   |*/
/*|        : ARC_CR_E_EVT: ���쌋�ʺ���ގ�M E			                   		   |*/
/*|        : ARC_CR_VER_EVT: �ް�ޮݎ�M 0x09			                   		   |*/
/*|        : 											                   		   |*/
/*|        :  0: 						 				                  		   |*/
/*|        : -1:Ireagual Data			 				                  		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Author      :  T.Nakayama                                              		   |*/
/*| Date        :  2004-05-19                                              		   |*/
/*| Update      :                                                          		   |*/
/*[]---------------------------------------------- Copyright(C) 2004 AMANO Corp.--[]*/
char	cr_queset( char seq )								/*						*/
{															/*						*/
	unsigned short evt=0;									/*						*/
															/*						*/
	if( seq == 0x04 ){										/* Eot					*/
															/*						*/
		md_seot( &FBcom_RcvData[2], 0 );
															/* 						*/
		/*--------------------------------------------------------------------------*/
		/* �}����������r�b�g��ON��OFF���̃C�x���g									*/
		/*--------------------------------------------------------------------------*/
		w_stat1[0] = w_stat1[1];							/*						*/
		w_stat1[1] = w_stat1[2];							/*						*/
		w_stat1[2] =										/* �}�������� New Status*/
			(unsigned char)( RED_stat[2] & TICKET_MOUTH );	/*						*/
															/*						*/
		/*--------------------------------------------------------------------------*/
		/* R�R�}���h��M�҂��C�x���g�AR�R�}���h��M�C�x���g							*/
		/*--------------------------------------------------------------------------*/
		if( w_stat2 == 0x00 ){								/* R����ގ�M�҂�?(Y)	*/
		   if( (RED_stat[3] & R_CMD_RCV) == R_CMD_RCV  ){	/* R����ގ�M�ς�?(Y)	*/
															/* 						*/
			queset( OPETCBNO, ARC_CR_EOT_RCMD, 0, NULL ); 	/* 24 					*/
			}												/*						*/
		}else{												/* R����ގ�M�ς�?(Y)	*/
		   if( (RED_stat[3] & R_CMD_RCV) == 0 ){			/* R����ގ�M�҂�		*/
															/* 						*/
			queset( OPETCBNO, ARC_CR_EOT_RCMD_WT, 0, NULL );/* 25 					*/
			}												/*						*/
		}													/*						*/
		w_stat2 =											/* R����ގ�M New Status*/
			(unsigned char)(RED_stat[3] & R_CMD_RCV);		/*						*/
															/*						*/
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//// MH321800(S) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
//		if(	( ( RED_stat[2] & TICKET_MOUTH) == TICKET_MOUTH ) &&//�}����������
//			( ( RED_stat[3] & 0x20) != 0x20 ) ){				//�J�[�h�}���҂��ł͂Ȃ�
//			if( pcard_shtter_ctl == 1 ){
//				queset( OPETCBNO, ARC_CR_EOT_MOVE_TKT, 0, NULL );	/* 30 			*/
//			}
//		}
//// MH321800(E) T.Nagai P�J�[�h�x���t�@�C���ɃV���b�^�[����s��Ή�(FCR.P170096)(MH341110���p)
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
															/*						*/
	}else{													/*						*/
		/*--------------------------------------------------------------------------*/
		/* TF4800N�̍\���̂ɁA���ȯĂŎ�M�����ް����߰����						*/
		/*--------------------------------------------------------------------------*/
		if( FBcom_RcvData[5] == FB_RCV_READ_ID){			/* D Read Data			*/
			evt = ARC_CR_R_EVT;								/* 20					*/
															/*						*/
			RDT_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RDT_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			memcpy( &RDT_REC.rdat[0],						/*						*/
					&FBcom_RcvData[5],						/*						*/
					(size_t)FBcom_RcvData[4]);				/*						*/
															/*						*/
															/*						*/
		}else if( FBcom_RcvData[5] == FB_RCV_END_ID){		/* E					*/
			evt = ARC_CR_E_EVT;								/* 21					*/
															/*						*/
			RED_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RED_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			RED_REC.ecod = FBcom_RcvData[5];				/* 						*/
			RED_REC.ercd = FBcom_RcvData[6];				/* Error Code			*/
															/*						*/
			RED_REC.posi[0] = FBcom_RcvData[7];				/* 						*/
			RED_REC.posi[1] = FBcom_RcvData[8];				/* 						*/
															/*						*/
															/*						*/
		}else if( FBcom_RcvData[5] == FB_RCV_VERS_ID){		/* Version				*/
			evt = ARC_CR_VER_EVT;							/* 22					*/
															/*						*/
			RVD_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RVD_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			RVD_REC.kind = FBcom_RcvData[5];				/*						*/
															/*						*/
			RVD_REC.vers[0] = FBcom_RcvData[6];				/* Ver					*/
			RVD_REC.vers[1] = FBcom_RcvData[7];				/* Ver					*/
															/*						*/
		}else if( (FBcom_RcvData[5] == FB_RCV_MNT_ID) &&	/* ����ݽ����މ���		*/
				  (FBcom_RcvData[6] == 0x01) ){				/* ����ݽ����=����		*/

			evt = ARC_CR_VER_EVT;							/* 22					*/
															/*						*/
			RVD_REC.idc1 = FBcom_RcvData[3];				/*						*/
			RVD_REC.idc2 = FBcom_RcvData[4];				/*						*/
															/*						*/
			RVD_REC.kind = FBcom_RcvData[5];				/*						*/
															/*						*/
			memcpy( RVD_REC.vers , &FBcom_RcvData[10] , 8);	/*						*/
			memcpy(MRD_VERSION, &FBcom_RcvData[10] , 8);	// ���C���[�_�[�o�[�W����
		}else if( (FBcom_RcvData[5] == FB_RCV_MNT_ID) &&	/* ����ݽ����މ���		*/
				  (FBcom_RcvData[6] == 0x09) ){				/* ����ݽ����=����		*/
			evt = ARC_CR_E_EVT;								/* 22					*/

			RDS_REC.result = FBcom_RcvData[9];				/* �擾����				*/
			RDS_REC.state[0] = FBcom_RcvData[10];			/* �f�B�b�v�X�C�b�`1���*/
			RDS_REC.state[1] = FBcom_RcvData[11];			/* �f�B�b�v�X�C�b�`2���*/
			RDS_REC.state[2] = FBcom_RcvData[12];			/* �f�B�b�v�X�C�b�`3���*/
			RDS_REC.state[3] = FBcom_RcvData[13];			/* �f�B�b�v�X�C�b�`4���*/

		}else{												/*						*/
			return (char)-1;								/*						*/
		}													/*						*/
															/*						*/
		if( evt != 0 ) {									/*						*/
			queset( OPETCBNO, evt, 0, NULL );				/* ������				*/
		}
															/*						*/
	}														/*						*/
	return(0);												/*						*/
}															/*						*/
															/*						*/
/*[]-------------------------------------------------------------------[]*/
/*|	Set Module Status ( EOT ) TF4800N									|*/
/*[]-------------------------------------------------------------------[]*/
/*| MODULE NAME  : md_seot( ptr, no );									|*/
/*| PARAMETER	 : char		*ptr	: Status Data Address				|*/
/*|		   int		no;	: Module No. ( 0 - 2 )							|*/
/*| RETURN VALUE : none													|*/
/*|              : RED_stat�X�V											|*/
/*[]-------------------------------------------------------------------[]*/
/*| Update	   : 2004.5.19 T.Nakayama									|*/
/*| 		   :	EOT�ð�������̒ǉ� mdltsk.c md_seot()				|*/
/*[]-------------------------------------------------------------------[]*/
static	void md_seot	( unsigned char *ptr, short no )
{
	unsigned char	*cp;
	short	size;
	unsigned char set;
	unsigned char alm;

	switch( no ) {
	    case 0:												/* ���Cذ�ް 			*/
		if( RD_mod < 6 ) {									/* �Ƽ�ٖ������͏���	*/
			return;											/*						*/
		}													/*						*/
		set = (unsigned char)prm_get( COM_PRM,S_PAY,21,1,3 );
		if(set == 1 || set == 2) {
			alm = ptr[0];
		}
		else {
			// �v�����^�Ȃ��̏ꍇ�̓v�����^�̃A���[�������O����
			alm = (unsigned char)(ptr[0] & 0x7f);
		}
		cp = RED_stat;										/* EOT status			*/
		if( cp[0] == 0 ) {									/*						*/
			if( alm != 0 ) {								/* Alarm Happen ?		*/
				alm_chk( ALMMDL_SUB, ALARM_READERSENSOR, 1 );/* �װ�۸ޓo�^����		*/
			}												/*						*/
		} else {											/*						*/
			if( alm == 0 ) {								/*						*/
				alm_chk( ALMMDL_SUB, ALARM_READERSENSOR, 0 );/* �װ�۸ޓo�^����		*/
			}												/*						*/
		}													/*						*/
		err_mod_chk( (uchar*)ptr, (uchar*)cp, 0 );			/* Ӽޭ�ً@��װ����	*/
															/* Reader				*/
		size = 4;											/* RED_stat size		*/
		break;												/*						*/
															/*						*/
	    default:
		return;
	}
	memcpy( cp, ptr, (size_t)size );						/* RED_stat�X�V			*/

	return;
}

/*[]------------------------------------------------------------------------------[]*/
/*|	���������p��M����															   |*/
/*|	���������pIBJ����ARCNET�f�[�^����M���A�f�[�^��������						   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| MODULE NAME  : avm_rcv()													   |*/
/*| PARAMETER	 : void															   |*/
/*| RETURN VALUE : none															   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| date	   : 2004.10.15 T.Nakayama											   |*/
/*| 		   : 																   |*/
/*[]------------------------------------------------------------------------------[]*/
void	avm_rcv( void )										/*						*/
{															/*						*/
	return;													/*						*/
}															/*						*/
															/*						*/
/*==================================================================================*/

/*[]------------------------------------------------------------------------------[]*/
/*| �������� ARCNET��M�ް��𷭰�־�Ă���                                          |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Argument : arc_txrx.rxARCdata[1]:���ݼ�ه�	                        		   |*/
/*|          : arc_txrx.rxARCdata[3]:�ް���ۯ���		                   		   |*/
/*|          : arc_txrx.rxARCdata[4]:�ް�����			                   		   |*/
/*|          : arc_txrx.rxARCdata[5]:�ް����			                   		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Return : queset											              		   |*/
/*|        : ARC_AVM_EXEC_EVT: �����������s/����		                   		   |*/
/*|        : ARC_AVM_VER_EVT:  �ް�ޮݎ�M				                   		   |*/
/*|        : 											                   		   |*/
/*|        :  0: 						 				                  		   |*/
/*|        : -1:Ireagual Data			 				                  		   |*/
/*[]------------------------------------------------------------------------------[]*/
/*| Author      :  T.Nakayama                                              		   |*/
/*| Date        :  2004-10-15                                              		   |*/
/*| Update      :                                                          		   |*/
/*[]---------------------------------------------- Copyright(C) 2004 AMANO Corp.--[]*/
char	avm_queset( char seq )								/*						*/
{															/*						*/
	return(0);												/*						*/
}															/*						*/
