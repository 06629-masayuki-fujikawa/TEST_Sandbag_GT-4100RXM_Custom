/************************************************************************************/
/*�@�֐����́@�@�@: ���Cذ�ނւ��ް����M����@�@�@�@�@		�@�@�@�@�@�@�@�@ �@		*/
/*�@�֐��V���{���@: cr_snd()  �@ �@�@: �@�@�@�@�@�@�@�@�@�@�@	�@�@�@�@�@�@: �@	*/
/*�@�R���p�C���@�@: MS-C ver.6.0�@	: �@�@�@�@�@�@�@�@�@�@�@�@�@	�@�@�@�@: �@	*/
/*�@�^�[�Q�b�gCPU : V25   �@�@�@�@�@: �@�@�@�@�@�@�@�@�@�@�@�@�@�@�@	�@�@: �@	*/
/*�@�쐬�ҁi���t�j: �˓c			  �@�@�@�@�@�@�@�@�@�@�@�@�@�@�@	�@�@: �@	*/
/*�@update:2004.9.16 T.Nakayama		  �@�@�@�@�@�@�@�@�@�@�@�@�@�@�@	�@�@: �@	*/
/************************************************************************************/
#include	<string.h>										/*						*/
#include	"system.h"										/*						*/
#include	"Message.h"										/* Message				*/
#include	"mdl_def.h"										/* Ӽޭ�يԒʐM�@��		*/
#include	"mem_def.h"										/*						*/
#include	"pri_def.h"										/*						*/
#include	"rkn_def.h"										/* �S�f�t�@�C������		*/
#include	"rkn_cal.h"										/*						*/
#include	"ope_def.h"										/*						*/
#include	"prm_tbl.h"										/*						*/
#include	"fbapi.h"
#include	"mdl_def.h"

unsigned char	MTS_tx[256] = {0};							/*						*/
															/*						*/
															/*						*/
int				MTS_tseq[MTS_MAX] = {0, 0};					/* ���ݼ�ه�			*/
															/*						*/
void	cr_snd( void )										/* ذ���ް����M			*/
{															/*						*/
	char bsiz;												/*						*/
															/*						*/
		/*--------------------------------------------------------------------------*/
		/* �}���������������؂̏ꍇ�A���ނ������������܂ő��M���Ȃ�				*/
		/*--------------------------------------------------------------------------*/
	if(( ( mts_req & MTS_BCRWT1 ) == MTS_BCRWT1 ) &&		/* �}��������������?(Y)	*/
	   ( ( RED_stat[2] & TICKET_MOUTH) == TICKET_MOUTH ) ){/* �}����������?(Y)	*/
															/*						*/
		return;												/*						*/
	}														/*						*/
															/*						*/
	mts_req = ( mts_req & ~MTS_BCRWT1 );					/* �v���ر				*/
															/*						*/
															/*						*/
		/*--------------------------------------------------------------------------*/
		/* ���M�v���`�F�b�N															*/
		/*--------------------------------------------------------------------------*/
	if(( mts_req & MTS_BCR1 ) == 0 )						/* ذ�ޑ��M�v���Ȃ�?	*/
	{														/*						*/
		return;												/*						*/
	}														/*						*/
															/*						*/
	mts_req = ( mts_req & ~MTS_BCR1 );						/* ���M�v���ر			*/
															/*						*/
	MTS_tx[0] = '2';										/* �]����(ذ��)			*/
	MTS_tx[1] = (char)( MTS_tseq[MTS_RED] | '0' );			/* ���ݼ�ه�			*/
	MTS_tseq[MTS_RED] = (char)(( MTS_tseq[MTS_RED] == 9 ) ?	/*						*/
							1 : ( MTS_tseq[MTS_RED] + 1 ));	/*						*/
	MTS_tx[2] = 0x02;										/* STX					*/
	MTS_tx[3] = 0x01;										/* ��ۯ���	(ID1)		*/
															/*						*/
	bsiz = MDLdata[MTS_RED].mdl_data.idc2;					/*						*/
															/*						*/
	MTS_tx[4] = bsiz;										/* ����		(ID2)		*/
	memcpy( &MTS_tx[5],										/*						*/
			&MDLdata[MTS_RED].mdl_data.rdat[0],				/*						*/
			(size_t)bsiz );									/*						*/
	MTS_tx[5 + bsiz] = 0x03;								/* ETX					*/
	MTS_tx[6 + bsiz] =										/* ����è			*/
		(unsigned char)(~( bcccal( (char*)&MTS_tx[3], (unsigned short)(bsiz+3) )));
															/*						*/
	FBcom_SetSendData(MTS_tx, (unsigned short)(bsiz + 7));
	
	// ���M�����ʒm
	queset( OPETCBNO, ARC_CR_SND_EVT, 0, NULL );
	return;													/*						*/
}															/*						*/
															/*						*/
															/*						*/
/*==================================================================================*/

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
void	avm_snd( void )										/* ذ���ް����M			*/
{															/*						*/
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
//char bsiz;												/*						*/
//	unsigned	char		ptr,exec;						/* write�|�C���^��Ɨp	*/
//	unsigned	char		count,msg_count;				/* ���b�Z�[�W�J�E���g�p */
//	unsigned	short		num;							/* 						*/
//	unsigned	short		ch;								/* ch�ԍ�				*/
	unsigned	char		count;							// ���b�Z�[�W�J�E���g�p
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
	struct		red_rec*	mdl;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
//	unsigned 	short		volume,ptn;						/* ���ʐݒ�				*/
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
	D_SODIAC_E	err;
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
//	ulong 	StartTime;
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
	mdl	 = &MDLdata[MTS_AVM].mdl_data;
		/*--------------------------------------------------------------------------*/
		/* ���M�v���`�F�b�N															*/
		/*--------------------------------------------------------------------------*/
															/*						*/
															/*						*/
	if(( mts_req & MTS_BAVM1 ) == 0 )						/* �����������M�v���Ȃ�?*/
	{														/*						*/
		return;												/*						*/
	}														/*						*/
															/*						*/
	mts_req = ( mts_req & ~MTS_BAVM1 );						/* �v���ر				*/
	MDLdata[MTS_AVM].mdl_size = 0;							/*		������c���Ă����Ȃ��ƃR�}���h�v�������Ȃ�				*/

	/*	���������G���[����̏ꍇ�͗v���������s��Ȃ� */
	if( SODIAC_ERR_NONE != Avm_Sodiac_Err_flg )
	{
		return;
	}
	/* �������牺�A��ARC�l�b�g�ɑ΂���o�^(M_A_DEFN)�E������(M_A_INIT)�EM_A_MSAG(���g�p)�R�}���h�͖�������B*/
	switch( mdl->rdat[0] )												/*	�v���R�}���h��ʂɂ�蕪��				�@	*/
	{																	/*                                            	*/
		case 0x0C:														/*  ������~�v���̏ꍇ                        	*/
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
//			ch = mdl->rdat[1];			
//			if( !AVM_Sodiac_Ctrl[ch].play_cmp ){
//				memset( &AVM_Sodiac_Ctrl[ch].sd_req[0].req_prm,	0x00, 
//					sizeof(AVM_SODIAC_CTRL));								/* �v���o�b�t�@all�N���A						*/
//				AVM_Sodiac_Ctrl[ch].play_cmp = 1;							//�����Đ���~���t���OON�i��~���j
//				AVM_Sodiac_Ctrl[ch].stop = 1;								// ��~�t���OON
//				AVM_Sodiac_Ctrl[ch].write_ptr = 0;							// �f�[�^�N���A
//				AVM_Sodiac_Ctrl[ch].read_ptr = 0;
//				AVM_Sodiac_Ctrl[ch].message_num = 0;
//
//				err	= sodiac_stop( ch );									/* �Đ���~�֐��R�[��							*/
//				if( D_SODIAC_E_OK != err )						
//				{
//					AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );			/* �G���[�o�^									*/
//				}
//				if( (ch == 0) && 											// ch0�w��
//					(!avm_alarm_flg) && 									// �x�񔭕񒆈ȊO
//					(AVM_Sodiac_Ctrl[1].message_num != 0) &&				// ch1���b�Z�[�W����
//					(AVM_Sodiac_Ctrl[1].play_message_cmp == 0) ) {			// ch1�Đ���
//					err	= sodiac_stop( 1 );									/* ch1�Đ���~�֐��R�[��							*/
//					if( D_SODIAC_E_OK != err ) {
//						AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );		/* �G���[�o�^									*/
//					}
//				}
//				if((LagChkExe( OPETCBNO, 22 ) != 0)||(LagChkExe( OPETCBNO, 23 ) != 0)){
//					// �I�y���[�V�������ŉ����̃C���^�[�o����(Sodiac�͒�~��)�ɒ�~�v�����󂯂��ꍇSodiac�͉�����Ԃ��Ȃ����ߋ����I������
//					Lagcan( OPETCBNO, 22 );										// �C���^�[�o���^�C�}��~
//					Lagcan( OPETCBNO, 23 );
//				}else{
//					Lagcan( OPETCBNO, 22 );										// �C���^�[�o���^�C�}��~
//					Lagcan( OPETCBNO, 23 );
//					// �L�����Z������������D_SODIAC_ID_PWMOFF����M����܂ő҂�
//					StartTime = LifeTim2msGet();
//					// NOTE:�v���O�����_�E�����[�h+rism�f�[�^�̑�ʑ��M��ԂŁA�N���W�b�g�̘A�����Z���s���ƁA���荞�݂ɍ����ׂ�������A������~�v����
//					// �����~����܂Ŏ��Ԃ�������P�[�X������B�^�C���A�E�g�ŋ����I��AVM_Sodiac_SemFree()�����s���A���ۂɉ����������W���[��������
//					// ��~���Ă��Ȃ���ԂŁA���O�������݂ɂ��FROM�̃A�N�Z�X����������ƁA�����������W���[�����C�u������sodiac_dataread()�Ŗ������[�v
//					// �Ɋׂ邽�ߑ΍􂷂�B
//					// �v���������ʍő�3.15s�̂��߃}�[�W�����2�{��6s�Ƃ���
//					while( 0 == LifePastTim2msGet(StartTime, 3000) ) {			// �ő�6s�ҋ@
//						taskchg( IDLETSKNO );
//						if( AVM_Sodiac_Ctrl[ch].stop == 0 ) {					// ��~���Ă��邩�iD_SODIAC_ID_PWMOFF��M�ς݁j
//							break;
//						}
//					}
//				}
//				if(AVM_Sodiac_Ctrl[ch].stop != 0) {
//					// ��莞�Ԍo�߂��Ă�stop == 1�ƂȂ�Ȃ��ꍇ�̓C���^�[�o�����̃L�����Z���ƍl����
//					// ���̏ꍇ�̓R�[���o�b�N��D_SODIAC_ID_PWMOFF����M���Ȃ��̂ŁA������SemFree���s��
//					AVM_Sodiac_Ctrl[ch].play_message_cmp = 1;				// ������~�t���OON
//					AVM_Sodiac_SemFree();
//					AVM_Sodiac_Ctrl[ch].stop = 0;
//				}
//			}
			// mdl->rdat[0] == 0x0C��an_stop()��set		mdl->rdat[1] = �I���`���l��
			// �A�i�E���X�I���v��
			if( PKTcmd_audio_end(
								0,			// �����I���`���l��(0�Œ�)
								0)			// ���f���@(0�Œ�)
				 == FALSE ){
				// error
			}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�I���v��
			break;
		case 0x0A:														/*  ���ʐݒ�v���̏ꍇ                        	*/
			// ����Đ��v�����Ɏw�肷��̂ŁA�s�v
			break;
		case 0x09:														/*	�e�X�g�R�}���h(M_A_TEST)					*/
			memset(&Avm_Sodiac_Info,0x00,sizeof(st_sodiac_version_info));
			err = sodiac_get_version_info( &Avm_Sodiac_Info );
			if( D_SODIAC_E_OK != err )
			{
				AVM_Sodiac_Err_Chk( err, ERR_SOIDAC_NOT_ID );		/* �G���[�o�^									*/
			}
			else
			{
				for( count = 0; count < 8; count++ )
				{
					/* �o�[�W�������̏I�[���擾	*/
					if( 0x00 == Avm_Sodiac_Info.sodiac_version[count] )
					{
						Avm_Sodiac_Info_end = count;
						break;
					}
				}	
			}
			break;
		case 0x0D:														/*	�����Đ��v���̏ꍇ							*/
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�J�n�v��
//			bsiz 		= mdl->idc2;									/* 	��ARCNET�ɐݒ肳�ꂽ�f�[�^�T�C�Y���擾����  */
//			ch	 		= mdl->rdat[ bsiz -2];
//			ptr	 		= AVM_Sodiac_Ctrl[ch].write_ptr;				/*  											*/
//			
//			/* �����ԍ����O�Ԃ�������99�ȏ�̏ꍇ�͔j������ */
//			if( !bsiz || (!mdl->rdat[1] && !mdl->rdat[2]) ){
//				break;
//			}
//			msg_count = (unsigned char)( bsiz - 5 );					/*	���b�Z�[�W�����Z�o	(msg_count/2)			*/
//
//			if( mdl->rdat[1] + mdl->rdat[2] == AVM_BOO ){				/* �x��										*/
//				volume = CPrmSS[S_SYS][60];
//			}
//			else{														/* �ʏ퉹										*/
//				ptn = get_timeptn();
//				if( 0xff == pre_volume[0])
//				{
//					volume = get_anavolume((uchar)ch, (uchar)ptn);		/* �ݒ肩�特�ʎ擾								*/
//				}
//				else
//				{
//					volume = pre_volume[0];								/*	�e�X�g���ʎ擾								*/
//					pre_volume[0] = 0xff;								/*	�e�X�g���ʃN���A							*/ 
//				}
//			}
//			if(volume == 0) {
//				return;								// ����0�Ȃ�������Ȃ�
//			}
//			else if(volume > 15) {
//				volume = 15;
//			}
//
//			--volume;
//			volume = 15 - volume;					// sodiac�ɑ΂��鉹��
//
//			/* ���b�Z�[�W�J��Ԃ����̂��߂Ƀ��b�Z�[�W�ԍ�������U�� */
//			if( 255 > AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num )
//			{
//				AVM_Sodiac_Ctrl[ch].message_num++;						/* ���b�Z�[�W�ԍ�����U��    */
//			}
//			else
//			{
//				AVM_Sodiac_Ctrl[ch].message_num = 1;				/* ���b�Z�[�W�ԍ�����U��    */
//			}
//			exec = 0;
//			for( count = 0; count < msg_count ; count+=2 )
//			{
//				if( (!mdl->rdat[ count+1] && !mdl->rdat[ count+2]) ){
//					continue;
//				}
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].resend_count		= (0x0F & mdl->rdat[ bsiz -4]);		/* �J��Ԃ��񐔎擾			 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].wait				= mdl->rdat[ bsiz -3];				/* �C���^�[�o���ݒ� �@		 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.ch			= mdl->rdat[ bsiz -2];				/* ch�ݒ� 					 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.kind 		= D_SODIAC_KIND_PHRASE_OUT;			/* �f�[�^�i�g�ݍ��킹�j�Đ��i�����΍��p�j	 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.option 		= D_SODIAC_OPTION_NONE;				/* option���ݒ�				 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.volume		= volume;							/* ����						 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.pitch		= 100;								/* �s�b�`�i��l�Œ�j		 */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.speed		= 100;								/* �X�s�[�h�i��l�Œ�j	 */
//				num													= mdl->rdat[ count+1];
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.num			= num<<8;							/* �����v���ԍ��ݒ�(���)�@  */
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].req_prm.num			|= mdl->rdat[ count+2];				/* �����v���ԍ��ݒ�(����)�@  */
//				
//
//				AVM_Sodiac_Ctrl[ch].sd_req[ptr].message_num 		= AVM_Sodiac_Ctrl[ch].message_num;
//
//				/* write�|�C���^�C���N�������g */	
//				if( AVM_Sodiac_Ctrl[ch].write_ptr >= (AVM_REQ_BUFF_SIZE -1) )
//				{
//					AVM_Sodiac_Ctrl[ch].write_ptr = 0;
//					ptr	= 0;
//				}
//				else
//				{
//					AVM_Sodiac_Ctrl[ch].write_ptr++;
//					ptr++;
//				}
//				exec++;
//			}
//			
//			if( !exec ){
//				return;																				/*								*/
//			}
//			AVM_Sodiac_Ctrl[ch].retry_count							= AVM_REQ_RETRY_MAX;			/* ���g���C�񐔁i�Q��j		 	*/
//
//			/* �����Đ���~���ł���΍Đ��v�����s���@*/
//			if( 1 == AVM_Sodiac_Ctrl[ch].play_cmp)
//			{
//				AVM_Sodiac_Ctrl[ch].resend_count_now	= AVM_Sodiac_Ctrl[ch].sd_req[AVM_Sodiac_Ctrl[ch].read_ptr].resend_count;	/* �c��đ��񐔐ݒ�	*/
//				/* �����Đ��v������			*/
//				AVM_Sodiac_Execute( ch );
//			}
			// mdl->rdat[0] == 0x0D��an_boo2()��set = AVM_BOO��AVM_IC_NG2�̎�
			// �A�i�E���X�J�n�v��
			if( PKTcmd_audio_start( 0, 1, (ushort)(mdl->rdat[1] + mdl->rdat[2]) ) == FALSE ){
				// error
			}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X(LCD_IF�Ή�)_�h�A�m�u�߂��Y��h�~�`���C��_�u�U�[��_�A�i�E���X�J�n�v��
			break;
		default:
			break;
	}
	return;													/*						*/
}															/*						*/
															/*						*/

