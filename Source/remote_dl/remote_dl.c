//[]----------------------------------------------------------------------[]
///	@mainpage		FTP���u�_�E�����[�h�Ή�
///	remote download function <br><br>
///	<b>Copyright(C) 2010 AMANO Corp.</b>
///	CREATE			2010/10/20 Namioka<br>
///	UPDATE			
///	@file			remote_dl.c
///	@date			2010/10/20
///	@version		MH759500

//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
#include	<string.h>
#include	<stddef.h>
#include	"system.h"										/*						*/
#include	"Message.h"										/* Message				*/
#include	"mem_def.h"										/*						*/
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"remote_dl.h"
#include	"mnt_def.h"
#include	"lcd_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"mdl_def.h"
#include	"flp_def.h"
#include	"ntnetauto.h"
#include	"raudef.h"
#include	"LKcom.h"
#include	"updateope.h"

// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
// /*------------------------------------------------------------------------------*/
// #pragma	section	_CHGINFO		/* "B":Uninitialized data area in external RAM2 */
// /*------------------------------------------------------------------------------*/
// // ���̃Z�N�V�����Ƀf�[�^��ǉ�����ꍇ�Ct_prog_chg_info���ɒǉ����邱�ƁI�I
// static	t_prog_chg_info	chg_info;
// static	uchar	work_update;
// static	t_prog_chg_info	bk_chg_info;
// static	t_remote_dl_info	remote_dl_info;
// static	t_remote_dl_info	bk_remote_dl_info;
// ushort	dummy_short;
// #pragma	section

// V0 ��MAP�ł̕���
//
//SECTION=B_CHGINFO
//  _chg_info
//  _bk_chg_info
//  _remote_dl_info
//  _bk_remote_dl_info

// V1 ��MAP�ł̕���
// SECTION=B_CHGINFO
//   _chg_info
//   _remote_dl_info
// SECTION=B_CHGINFO_BKUP
//   _bk_chg_info
//   _bk_remote_dl_info


/*------------------------------------------------------------------------------*/
#pragma	section	_CHGINFO		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// ���̃Z�N�V�����Ƀf�[�^��ǉ�����ꍇ�Ct_prog_chg_info���ɒǉ����邱�ƁI�I
static	uchar	work_update;
ushort	dummy_short;
static	t_prog_chg_info	chg_info;
static	t_remote_dl_info	remote_dl_info;
#pragma	section
/*------------------------------------------------------------------------------*/
#pragma	section	_CHGINFO_BKUP		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// ���̃Z�N�V�����Ƀf�[�^��ǉ�����ꍇ�Ct_prog_chg_info���ɒǉ����邱�ƁI�I
static	t_prog_chg_info	bk_chg_info;
static	t_remote_dl_info	bk_remote_dl_info;
#pragma	section
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���

static	uchar	task_status;
static	t_FtpInfo	g_bk_FtpInfo;
t_NtBufCount		g_bufcnt;
static const uchar st_offset[2][6] = {
	{ 8, 6, 5, 2, 1,14 },
	{ 7,18,16, 4, 3, 9 },
};

const uchar ReqAcceptTbl[REMOTE_REQ_MAX] = {
// GG124100(S) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6586 �N�����ANT-NET�d���@�Z���^�[�p�[�����f�[�^(ID�F065)�̍��ځu��t�\�v���v�ɑΉ����Ă��Ȃ��@�\���u��t�v�ɂ��đ��M���� [���ʉ��P���� No1531]
// 	0,
// 	1,			// �o�[�W�����A�b�v�v����t
// 	0,			// ���ԕύX�v��
// // MH810100(S)
// //	0,			// �ݒ�ύX�v����t
// //	0,			// �ݒ�v����t
// 	1,			// �ݒ�ύX�v����t
// 	1,			// �ݒ�v����t
// // MH810100(E)
// 	1,			// ���Z�b�g�v����t
// 	0,			// �v���O�����؊��v����t
// 	1,			// FTP�ݒ�ύX�v����t
// 	0,			// ���u�����ݒ��t
// // MH810100(S) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
// 	0,			// �\��
// 	0,			// FTP�ڑ��e�X�g�v��
// 	0,			// �\��
// 	0,			// �\��
// 	1,			// �ݒ�ύX�v��
// // MH810100(E) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	0,
// GG129000(S) R.Endo 2022/11/15 �Ԕԃ`�P�b�g���X4.0 #6690 phase9�i�v���O�����A�b�v�f�[�g���j�Ή�
// 	0,			// �o�[�W�����A�b�v�v��
// 	0,			// ���ԕύX�v��
// 	0,			// �ݒ�ύX�v��
// 	1,			// �ݒ�v��(�A�b�v���[�h)
// 	0,			// ���Z�b�g�v��
// 	0,			// �v���O�����ؑ֗v��
// 	0,			// FTP�ݒ�ύX�v��
// 	0,			// ���u�����ݒ�v��
// 	0,			// �\��
// 	0,			// FTP�ڑ��e�X�g�v��
// 	0,			// �\��
// 	0,			// �\��
// 	0,			// �����ݒ�ύX�v��
	1,			// �o�[�W�����A�b�v�v��
	0,			// ���ԕύX�v��
	1,			// �ݒ�ύX�v��
	1,			// �ݒ�v��(�A�b�v���[�h)
	1,			// ���Z�b�g�v��
	0,			// �v���O�����ؑ֗v��
	1,			// FTP�ݒ�ύX�v��
	0,			// ���u�����ݒ�v��
	0,			// �\��
	1,			// FTP�ڑ��e�X�g�v��
	0,			// �\��
	0,			// �\��
	1,			// �����ݒ�ύX�v��
// GG129000(E) R.Endo 2022/11/15 �Ԕԃ`�P�b�g���X4.0 #6690 phase9�i�v���O�����A�b�v�f�[�g���j�Ή�
// GG124100(E) R.Endo 2022/09/08 �Ԕԃ`�P�b�g���X3.0 #6586 �N�����ANT-NET�d���@�Z���^�[�p�[�����f�[�^(ID�F065)�̍��ځu��t�\�v���v�ɑΉ����Ă��Ȃ��@�\���u��t�v�ɂ��đ��M���� [���ʉ��P���� No1531]
};


#define TEST_CONNECT_FILE_SIZE		1024

#define FILENAME_PARAM_UP			0
#define FILENAME_PARAM_DOWN			1
#define FILENAME_TEST				2
static void MakeRemoteFileNameForIP_MODEL_NUM(ushort UpDown,char *remote);
// MH810103(s) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��
//// MH322915(S) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
//static eVER_TYPE GetVersionPrefix( uchar *buf, ushort size );
//// MH322915(E) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
// MH810103(e) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
static uchar remotedl_request_kind_to_connect_type(int request);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
static ushort	g_usRmonSeq = 0;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
static uchar connect_type_to_dl_time_kind(uchar connect_type);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
static long remotedl_nrm_time_sub(ulong from, ulong to);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�^�X�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_task( void )								
{															
	while(1){												
		taskchg( IDLETSKNO );								
		if( TASK_START_FOR_DL() ){							// Task�N���v������
// MH810100(S) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
			// LCD�ւ̃p�����[�^�A�b�v���[�h����r��
			if( OPECTL.lcd_prm_update ){
				continue;
			}
// MH810100(E) K.Onodera  2020/03/31 #4098 �Ԕԃ`�P�b�g���X(�v���O�����_�E�����[�h���̓d��OFF/ON�ōĊJ���s)
			ftp_remote_auto_update();						// FTP�ʐM������
		}
	}
}															
															
//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�^�X�N ���b�Z�[�W�擾����
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			comd	:�C�x���g
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
ushort	GetMessageRemote( void )							
{															
															
	MsgBuf	*msb;
	ushort 	comd;

	for( ; ; ) {
		taskchg( IDLETSKNO );								
		if( (msb = GetMsg( REMOTEDLTCBNO )) == NULL ){
			continue;
		}
		comd = msb->msg.command;
		break;
	}
	FreeBuf( msb );
	return( comd );

}															

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h��ԍX�V
//[]----------------------------------------------------------------------[]
///	@param[in]		sts		:FTP�ʐM���	
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_status_set( uchar sts )					
{															
	chg_info.status = sts;
	task_status = sts;
}															

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h��ԎQ��
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:FTP�ʐM���
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_status_get( void )								
{															
	return	(chg_info.status);
}															

//[]----------------------------------------------------------------------[]
///	@brief			�^�X�N�N���p���u�_�E�����[�h��ԎQ��
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:FTP�ʐM���(�^�X�N�ؑ֎��p�̏������G���A���g�p)
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_task_startup_status_get( void )
{															
	return	(task_status);
}															

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�ڑ���ʐݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		type	:�ڑ�/�ؒf���
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_connect_type_set( uchar type )	
{												
	chg_info.connect_type = type;
}												

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�ڑ���ʎ擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			connect_type	:�ڑ�/�ؒf���
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_connect_type_get( void )
{										
	return	(chg_info.connect_type);
}										

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			t_prog_chg_info	:�_�E�����[�h���|�C���^
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
t_prog_chg_info*	remotedl_info_get( void )				
{															
	return (&chg_info);
}															

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_info_clr( void )							
{															
	memset( &chg_info, 0, sizeof(chg_info));
	remotedl_result_clr();
	memset(&remote_dl_info, 0, sizeof(remote_dl_info));
}															

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���ʍX�V
//[]----------------------------------------------------------------------[]
///	@param[in]		result	:���ʎ��
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_result_set( uchar result )
{
	uchar	index = 0;
	if( !DOWNLOADING() ){
		return;
	}
	switch (remotedl_connect_type_get()) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//	case CTRL_PROG_DL:
//	case CTRL_PARAM_DL:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//	case CTRL_PARAM_DIF_DL:			//�����ݒ�A�b�v���[�h
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//	case CTRL_PROG_SW:
//		if ((remotedl_status_get() == R_SW_WAIT) ||
//			(remotedl_status_get() == R_SW_START)) {
//			index = RES_SW;
//		}
//		else {
//			index = RES_DL;
//		}
//		break;
	case CTRL_PROG_DL:
	case CTRL_PROG_SW:
		if ((remotedl_status_get() == R_SW_WAIT) ||
			(remotedl_status_get() == R_SW_START)) {
			index = RES_SW_PROG;
		}
		else {
			index = RES_DL_PROG;
		}
		break;
	case CTRL_PARAM_DL:
		if ((remotedl_status_get() == R_SW_WAIT) ||
			(remotedl_status_get() == R_SW_START)) {
			index = RES_SW_PARAM;
		}
		else {
			index = RES_DL_PARAM;
		}
		break;
	case CTRL_PARAM_DIF_DL:			//�����ݒ�A�b�v���[�h
		if ((remotedl_status_get() == R_SW_WAIT) ||
			(remotedl_status_get() == R_SW_START)) {
			index = RES_SW_PARAM_DIF;
		}
		else {
			index = RES_DL_PARAM_DIF;
		}
		break;
// GG120600(E) // Phase9 ���ʂ���ʂ���
	case CTRL_PARAM_UPLOAD:
		index = RES_UP;
		break;
	case CTRL_CONNECT_CHK:
		index = RES_COMM;
		break;
// GG120600(S) // Phase9 LCD�p
	case CTRL_PARAM_UPLOAD_LCD:
		index = RES_UP_LCD;
		break;
// GG120600(E) // Phase9 LCD�p
	default:
		return;
	}
	chg_info.result[index] = result;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���ʎ擾
//[]----------------------------------------------------------------------[]
///	@param[in]		type	:�擾���錋�ʎ��
///	@return			result	:�Ώۂ̌���
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_result_get( uchar type )
{
	return (chg_info.result[type]);
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���ʏ�ԏ�����
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			none
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_result_clr( void )
{
	memset( chg_info.result, EXCLUDED, sizeof(chg_info.result));	// ���ʃR�[�h��0xff�ŏ�����
}

//[]----------------------------------------------------------------------[]
///	@brief			�v���O�����X�V���ʐݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_update_set( void )	
{
	chg_info.update = 1;				// �v���O�����X�V�����t���O���Z�b�g
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h/���ʏ�񃊃g���C���ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar retry_info_set(uchar kind)
{
	uchar type = remotedl_status_get();
	uchar ret = 0;
	ulong wtime,interval,overtime;

	if( chg_info.retry[kind].status == RETRY_OVER ){	// ���g���C�񐔂��I�[�o�[���Ă���ꍇ�́A���g���C���Ȃ�
		return 1;
	}

// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	if (kind == RETRY_KIND_CONNECT) {
	if ( kind < RETRY_KIND_CONNECT_MAX) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		interval = prm_get(COM_PRM, S_RDL, 5, 3, 3);
		memset( &chg_info.retry[kind].time, 0, sizeof( date_time_rec ));
	}
	else {
		// ���g���C�Ԋu�i���j
		switch (kind) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		case RETRY_KIND_DL:
//		case RETRY_KIND_UL:
		case RETRY_KIND_DL_PRG:
		case RETRY_KIND_DL_PARAM:
		case RETRY_KIND_DL_PARAM_DIFF:
		case RETRY_KIND_UL:
// GG120600(E) // Phase9 ���ʂ���ʂ���
			interval = prm_get(COM_PRM, S_RDL, 1, 3, 3);
			break;
		default:
			interval = 0;
			break;
		}

		memcpy( &chg_info.retry[kind].time, &CLK_REC, sizeof( date_time_rec ));
		wtime = Nrm_YMDHM( &chg_info.retry[kind].time );

		overtime = ( wtime&0x0000ffff ) + (interval%1440);
		if( overtime > 1439 ){							// �C���^�[�o����̎��Ԃ����ׂ�����ꍇ
			wtime += 0x10000;							// �{�P������
			wtime = ((wtime&0xffff0000) + (overtime - 1440));	// �z������̎���(��)��ݒ肷��
		}else{											// ���ׂ����Ȃ��ꍇ
			wtime += ((interval%1440));
		}

		wtime += ((interval/1440) << 16);				// �����Ԋu���{����
		UnNrm_YMDHM( &chg_info.retry[kind].time, wtime );
	}

	// ���g���C��
	if (chg_info.retry[kind].count == 0) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//		if (kind == RETRY_KIND_CONNECT) {
		if ( kind < RETRY_KIND_CONNECT_MAX) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			chg_info.retry[kind].count = prm_get(COM_PRM, S_RDL, 5, 2, 1);
		}
		else {
			switch (kind) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//			case RETRY_KIND_DL:
			case RETRY_KIND_DL_PRG:
			case RETRY_KIND_DL_PARAM:
			case RETRY_KIND_DL_PARAM_DIFF:
// GG120600(E) // Phase9 ���ʂ���ʂ���
			case RETRY_KIND_UL:
				chg_info.retry[kind].count = prm_get(COM_PRM, S_RDL, 1, 2, 1);
				break;
			default:
				chg_info.retry[kind].count = 0;
				break;
			}
		}
	}

	// ���g���C����
	if( !chg_info.retry[kind].status ){					// ����̏ꍇ
		if( chg_info.retry[kind].count ){				// �񐔂��O�ȊO�Ȃ�΁A
			chg_info.retry[kind].status = type;			// ���g���C�\��Z�b�g
		}else{											// �񐔂��O��̏ꍇ��
			chg_info.retry[kind].status = RETRY_OVER;	// ���g���C�I�������g���C�ς݂��Z�b�g
			ret = 1;									// ���g���C�I�[�o�[�Ƃ���
		}
	}else{												// ���g���C��
		if( --chg_info.retry[kind].count == 0 ){		// ���g���C�񐔎c��Ȃ�
			chg_info.retry[kind].status = RETRY_OVER;	// ���g���C�I�������g���C�ς݂��Z�b�g
			ret = 1;									// ���g���C�I�[�o�[
		}else{
			chg_info.retry[kind].status &= 0x0f;		// ���g���C���t���OOFF
		}
	}

	// ���g���C�I�[�o�[�ł͂Ȃ��H
	if (ret != 1) {
		if( !chg_info.pow_flg ) {						// ���d�t���O��OFF
			if( !interval ){							// �C���^�[�o���Ԋu���O���̏ꍇ�͑�������
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				if (kind == RETRY_KIND_CONNECT) {
				if ( kind < RETRY_KIND_CONNECT_MAX) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					memcpy( &chg_info.retry[kind].time, &CLK_REC, sizeof( date_time_rec ));
				}
				queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
			}
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			else if (kind == RETRY_KIND_CONNECT) {		// �ڑ����g���C�̏ꍇ�A�^�C�}�𓮍삳����
//				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer);
			else if (kind ==  RETRY_KIND_CONNECT_PRG) {		// �ڑ����g���C�̏ꍇ�A�^�C�}�𓮍삳����
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer);
			}else if (kind == RETRY_KIND_CONNECT_PARAM_UP) {		// �ڑ����g���C�̏ꍇ�A�^�C�}�𓮍삳����
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer2);
			}else if (kind == RETRY_KIND_CONNECT_PARAM_DL) {		// �ڑ����g���C�̏ꍇ�A�^�C�}�𓮍삳����
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer3);
			}else if (kind == RETRY_KIND_CONNECT_PARAM_DL_DIFF) {		// �ڑ����g���C�̏ꍇ�A�^�C�}�𓮍삳����
				LagTim500ms(LAG500_REMOTEDL_RETRY_CONNECT_TIMER, interval*2, retry_info_connect_timer4);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			}
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�ڑ����g���C�J�n
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-02-06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void retry_info_connect_timer(void)
{
	// ���d�t���OOFF
	remotedl_pow_flg_set(FALSE);
	// �ڑ��J�n�����Z�b�g
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	memcpy( &chg_info.retry[RETRY_KIND_CONNECT].time, &CLK_REC, sizeof( date_time_rec ));
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PRG].time, &CLK_REC, sizeof( date_time_rec ));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
void retry_info_connect_timer2(void)
{
	// ���d�t���OOFF
	remotedl_pow_flg_set(FALSE);
	// �ڑ��J�n�����Z�b�g
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PARAM_UP].time, &CLK_REC, sizeof( date_time_rec ));
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
void retry_info_connect_timer3(void)
{
	// ���d�t���OOFF
	remotedl_pow_flg_set(FALSE);
	// �ڑ��J�n�����Z�b�g
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PARAM_DL].time, &CLK_REC, sizeof( date_time_rec ));
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
void retry_info_connect_timer4(void)
{
	// ���d�t���OOFF
	remotedl_pow_flg_set(FALSE);
	// �ڑ��J�n�����Z�b�g
	memcpy( &chg_info.retry[RETRY_KIND_CONNECT_PARAM_DL_DIFF].time, &CLK_REC, sizeof( date_time_rec ));
	queset( OPETCBNO, REMOTE_RETRY_SND, 0, NULL );
}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���g���C�����N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void retry_info_clr(uchar kind)
{
	if (kind == RETRY_KIND_MAX) {
		memset(&chg_info.retry, 0, sizeof(chg_info.retry));
	}
	else {
		memset(&chg_info.retry[kind], 0, sizeof(t_retry_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���g���C�J�E���g���N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-02-06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void retry_count_clr(uchar kind)
{
	int i;

	if (kind == RETRY_KIND_MAX) {
		for (i = 0; i < RETRY_KIND_MAX; i++) {
			chg_info.retry[i].count = 0;
		}
	}
	else {
		chg_info.retry[kind].count = 0;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���g���C���Ԃ��N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-02-06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void retry_time_clr(uchar kind)
{
	int i;

	if (kind == RETRY_KIND_MAX) {
		for (i = 0; i < RETRY_KIND_MAX; i++) {
			memset(&chg_info.retry[i].time, 0, sizeof(date_time_rec));
		}
	}
	else {
		memset(&chg_info.retry[kind].time, 0, sizeof(date_time_rec));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�X�V�������ʎ擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			update	:0�F�v���O�����X�V������ 1�F�v���O�����X�V�ς�
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_update_chk( void )	
{
	uchar ret = 0;

	if( ( TENKEY_F1 == 1 )&&( TENKEY_F3 == 1 ) || 
		( TENKEY_F3 == 1 )&&( TENKEY_F5 == 1 )){
		work_update = 0;
	}else{
		// ���u�����e�i���X�����o�b�N�A�b�v
		memcpy(&bk_remote_dl_info, &remote_dl_info, sizeof(remote_dl_info));
		if( chg_info.update == 1 ){
			work_update = chg_info.update;
			// �v���O�����X�V�A�ݒ�X�V���̓t���O���N���A����
			f_ParaUpdate.BYTE = 0;
			if (remotedl_connect_type_get() == CTRL_PROG_DL ||
				remotedl_connect_type_get() == CTRL_PROG_SW) {
				ret = 1;
			}
		}else{
			work_update = 0;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�X�V�������ʎ擾�iMAF�����ݒ�p�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			update	:0�F�v���O�����X�V������ 1�F�v���O�����X�V�ς�
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_first_comm_get( void )	
{
	uchar ret = work_update;

	work_update = 0;

	return 	(ret);
}

// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
uchar	remotedl_work_update_get( void )	
{
	uchar ret = work_update;

	return 	(ret);
}
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���X�g�A�������s
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_restore( void )							
{															
	t_SysMnt_ErrInfo	errinfo;
	uchar	status,i;
	ulong 	ret;
	uchar	timerset = 0;
	uchar	result;
	uchar	connect = remotedl_connect_type_get();
	uchar	comp = EXEC_STS_NONE;
	PRG_HDR	header;
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
	uchar	kind;
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

	if( work_update ){
		remotedl_status_set( R_SW_START );
		
		// �v���O�����؊����̓��X�g�A���s��
		remotedl_chg_info_restore();
		memcpy(&remote_dl_info, &bk_remote_dl_info, sizeof(bk_remote_dl_info));
		connect = remotedl_connect_type_get();
		if (connect == CTRL_PROG_DL ||
			connect == CTRL_PROG_SW) {
			for( i=0; i<RESTORE_RETRY_COUNT; i++){
				errinfo.errmsg = _SYSMNT_ERR_NONE;
				sysmnt_Restore(&errinfo);
				if( errinfo.errmsg == _SYSMNT_ERR_NONE ){
					break;
				}
				WACDOG;
			}
			for( i=0; i<RESTORE_RETRY_COUNT; i++){
				ret = FLT_Restore_FLAPDT();
				if( _FLT_RtnKind(ret) == FLT_NORMAL ){
					dsp_background_color(COLOR_BLACK);
					grachr( 7, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[66] );	// "       ۯ����u���ؽı        ",�\��
					dsp_background_color(COLOR_WHITE);
					break;
				}
				WACDOG;
			}
			wopelg( OPLOG_ALLRESTORE, 0, 0 );			// ���엚��o�^
			if ((errinfo.errmsg == _SYSMNT_ERR_NONE) && (_FLT_RtnKind(ret) == FLT_NORMAL)) {
				remotedl_result_set(PROG_SW_COMP);		// �v���O�����X�V����
			}
			else {
				remotedl_result_set(RESTORE_ERR);		// ���X�g�A���s
			}
		}
		else {
			remotedl_result_set(PROG_SW_COMP);			// �X�V����
		}
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		ret = remotedl_result_get(RES_SW);
		switch (connect) {
		case CTRL_PROG_DL:
		case CTRL_PROG_SW:
			ret = remotedl_result_get(RES_SW_PROG);
			break;
		case CTRL_PARAM_DL:
			ret = remotedl_result_get(RES_SW_PARAM);
			break;
		case CTRL_PARAM_DIF_DL:							// �����ݒ�A�b�v���[�h
			ret = remotedl_result_get(RES_SW_PARAM_DIF);
			break;
		}
// GG120600(E) // Phase9 ���ʂ���ʂ���
		moitor_regist(OPLOG_REMOTE_SW_END, ret);		// �X�V�����̑��샂�j�^��o�^
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//		remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
		switch (connect) {
		case CTRL_PROG_DL:
		case CTRL_PROG_SW:
			switch (ret) {
			case PROG_SW_COMP:
				rmon_regist(RMON_PRG_SW_END_OK);
				comp = EXEC_STS_COMP;
				break;
			case RESTORE_ERR:
				rmon_regist(RMON_PRG_SW_END_RESTORE_NG);
				break;
			}

			if (connect == CTRL_PROG_DL) {
				remotedl_complete_request(REQ_KIND_VER_UP);
				remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, comp);
			}
			else {
				remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
				remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, comp);
			}
			// �ċN��
			if (ret == PROG_SW_COMP) {
				System_reset();						// Main CPU reset (It will not retrun from this function)
			}
			break;
		case CTRL_PARAM_DL:
			rmon_regist(RMON_PRM_SW_END_OK);

			remotedl_complete_request(REQ_KIND_PARAM_CHG);
			remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_COMP);
			break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_DIF_DL:							// �����ݒ�A�b�v���[�h
			rmon_regist(RMON_PRM_SW_DIF_END_OK);		// ���u�Ď��f�[�^�o�^

			remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);		// ���u�����e�i���X�v������
			remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_COMP);		// ���u�����e�i���X�����X�e�[�^�X�ݒ�
			break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		default:
			break;
		}
		return timerset;
	}
	
	status = remotedl_status_get();
	switch( status ){
		case	R_DL_START:
		case	R_DL_EXEC:
		case	R_SW_START:
			result = (uchar)(remotedl_exec_info_get()+PROG_DL_RESET);
			remotedl_result_set( result );						// ���ʏ����Z�b�g�i���Z�b�g�ɂ�鏈�����~�j
			moitor_regist(( status == R_SW_START?OPLOG_REMOTE_SW_END:OPLOG_REMOTE_DL_END ), result );	// �_�E�����[�h/�X�V�����i�ُ�j�̑��샂�j�^��o�^
			switch (status) {
			case R_DL_START:		// ���u�_�E�����[�h���s�J�n�i�\��j
			case R_DL_EXEC:			// ���u�_�E�����[�h���s�iTask�N���j
			case R_UP_START:		// �A�b�v���[�h���s�J�n
				// ���d�t���OON
				remotedl_pow_flg_set(TRUE);
				// write�����N���A
				remotedl_write_info_clear();

				// ���g���C�J�E���g���N���A
				if (connect == CTRL_PARAM_UPLOAD) {
					retry_info_clr(RETRY_KIND_UL);
				}
				else {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//					retry_info_clr(RETRY_KIND_DL);
					switch (connect) {
					case CTRL_PARAM_DL:
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
						break;
					case CTRL_PARAM_DIF_DL:
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
						break;
					}
// GG120600(E) // Phase9 ���ʂ���ʂ���
				}
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_count_clr(RETRY_KIND_CONNECT);
				switch (connect) {
				case CTRL_PROG_DL:
					kind = RETRY_KIND_CONNECT_PRG;
					break;
				case CTRL_PARAM_DL:
					kind = RETRY_KIND_CONNECT_PARAM_DL;
					break;
				case CTRL_PARAM_DIF_DL:
					kind = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
					break;
				case CTRL_PARAM_UPLOAD:
					kind = RETRY_KIND_CONNECT_PARAM_UP;
					break;
				default:
					break;
				}
				retry_count_clr(kind);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

				// �ڑ����g���C
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				if (retry_info_set(RETRY_KIND_CONNECT)) {
				if (retry_info_set(kind)) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					// ���g���C���N���A
//					retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					// �����̃��[�g�̓��g���C�Ȃ��������肦�Ȃ�
					// ���g���C�I�[�o�[�Ȃ̂Ŏ󂯕t�����v�����N���A����
					switch (connect) {
					case CTRL_PROG_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
						retry_info_clr(RETRY_KIND_CONNECT_PRG);
						retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						rmon_regist(RMON_PRG_DL_END_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						break;
					case CTRL_PARAM_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
						retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						rmon_regist(RMON_PRM_DL_END_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
						retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						rmon_regist(RMON_PRM_DL_DIF_END_RETRY_OVER);		// ���u�Ď��f�[�^�o�^
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// ���u�����e�i���X�v������
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					case CTRL_PARAM_UPLOAD:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
						retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
						retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						rmon_regist(RMON_PRM_UP_END_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						remotedl_comp_set(INFO_KIND_START, REQ_KIND_PARAM_UL, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
						break;
					default:
						break;
					}
				}
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
				// FROM�̃w�b�_�����폜���Ă���
				if (connect == CTRL_PROG_DL) {
					memset(&header, 0xFF, sizeof(header));
					FLT_write_program_version( (uchar*)&header );
				}
// GG129000(S) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
				remotedl_arrange_next_request();
// GG129000(E) R.Endo 2023/01/11 �Ԕԃ`�P�b�g���X4.0 #6774 �v���O�����_�E�����[�h�������̓d���f�ōċN����Ƀ��g���C���� [���ʉ��P���� No1537]
				// Lagtim��������Ƀ^�C�}�Z�b�g����
				timerset = 1;
				break;
			case R_SW_START:		// �v���O�����X�V�J�n
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
				switch (connect) {
				case CTRL_PROG_DL:
				case CTRL_PROG_SW:
					rmon_regist(RMON_PRG_SW_END_RESET_NG);			// �X�V���Ƀ��Z�b�g
					if (connect == CTRL_PROG_DL) {
						remotedl_complete_request(REQ_KIND_VER_UP);
					}
					else {
						remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
					}
					break;
				case CTRL_PARAM_DL:
					rmon_regist(RMON_PRM_SW_END_RESET_NG);			// �X�V���Ƀ��Z�b�g
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case CTRL_PARAM_DIF_DL:
					rmon_regist(RMON_PRM_SW_DIF_END_RESET_NG);			// �X�V���Ƀ��Z�b�g
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// ���u�����e�i���X�v������
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				default:
					break;
				}
				break;
			default:
				break;
			}
			break;


		case	R_DL_IDLE:
				// ���d�t���OON
				remotedl_pow_flg_set(TRUE);
				// ���g���C���ł���΃J�E���g�N���A
				if (now_retry_active_chk(RETRY_KIND_MAX)) {
					// ���g���C�J�E���g���N���A
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					retry_info_clr(RETRY_KIND_DL);
//					retry_info_clr(RETRY_KIND_UL);
//					retry_count_clr(RETRY_KIND_CONNECT);
					// ���ׂăN���A
					retry_count_clr(RETRY_KIND_MAX);
					switch (connect) {
					case CTRL_PROG_DL:
						kind = RETRY_KIND_CONNECT_PRG;
						break;
					case CTRL_PARAM_DL:
						kind = RETRY_KIND_CONNECT_PARAM_DL;
						break;
					case CTRL_PARAM_DIF_DL:
						kind = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
						break;
					case CTRL_PARAM_UPLOAD:
						kind = RETRY_KIND_CONNECT_PARAM_UP;
						break;
					default:
						break;
					}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					// �ڑ����g���C
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					if (retry_info_set(RETRY_KIND_CONNECT)) {
					if (retry_info_set(kind)) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//						// ���g���C���N���A
//						retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
						// �����̃��[�g�̓��g���C�Ȃ��������肦�Ȃ�
						// ���g���C�I�[�o�[�Ȃ̂Ŏ󂯕t�����v�����N���A����
						switch (remotedl_connect_type_get()) {
						case CTRL_PROG_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
							retry_info_clr(RETRY_KIND_CONNECT_PRG);
							retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
							rmon_regist(RMON_PRG_DL_END_RETRY_OVER);
							remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							break;
						case CTRL_PARAM_DL:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
							retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
							retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
							rmon_regist(RMON_PRM_DL_END_RETRY_OVER);
							remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
						case CTRL_PARAM_DIF_DL:		//�����ݒ�A�b�v���[�h
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
							retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
							retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
							rmon_regist(RMON_PRM_DL_DIF_END_RETRY_OVER);		// ���u�Ď��f�[�^�o�^
							remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// ���u�����e�i���X�v������
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
						case CTRL_PARAM_UPLOAD:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
							retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
							retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
							rmon_regist(RMON_PRM_UP_END_RETRY_OVER);
							remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5820:FTP���g���C���ɐڑ����g���C�񐔂�0�ɕύX��A�d��OFF/ON�Ń��g���C�I�[�o�[���Ȃ�)
							break;
						default:
							break;
						}
					}
					else {
						// Lagtim��������Ƀ^�C�}�Z�b�g����
						timerset = 1;
					}
				}
				else {
					retry_info_clr(RETRY_KIND_MAX);
				}
				// write�����N���A
				remotedl_write_info_clear();
				break;
		case	R_RESET_START:
				rmon_regist(RMON_RESET_END_OK);
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
				remotedl_complete_request(REQ_KIND_RESET);
				remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_COMP);
				break;
		case	R_TEST_CONNECT:
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
				remotedl_complete_request(REQ_KIND_TEST);
				break;
		case	R_DL_REQ_RCV:
		case	R_UP_WAIT:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				// �J�n���胊�g���C
				if (remotedl_dl_start_retry_check() < 0) {
					// �����̃��[�g�̓��g���C�Ȃ��������肦�Ȃ�
					// ���g���C�I�[�o�[
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					switch (remotedl_connect_type_get()) {
					case CTRL_PROG_DL:
						rmon_regist(RMON_PRG_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
						remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
						break;
					case CTRL_PARAM_DL:
						rmon_regist(RMON_PRM_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					case CTRL_PARAM_DIF_DL:		// �����ݒ�A�b�v���[�h
						rmon_regist(RMON_PRM_DL_DIF_START_RETRY_OVER);		// ���u�Ď��f�[�^�o�^
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);	// ���u�����e�i���X�v������
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);		// ���u�����e�i���X�����X�e�[�^�X�ݒ�
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					case CTRL_PARAM_UPLOAD:
						rmon_regist(RMON_PRM_UP_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_UL);
						remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
						break;
					default:
						break;
					}
				}
				break;
		case	R_SW_WAIT:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				if (remotedl_sw_start_retry_check() < 0) {
					// �����̃��[�g�̓��g���C�Ȃ��������肦�Ȃ�
					// ���g���C�I�[�o�[
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					switch (remotedl_connect_type_get()) {
					case CTRL_PROG_DL:
						rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
						remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
						break;
					case CTRL_PARAM_DL:
						rmon_regist(RMON_PRM_SW_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
						remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					case CTRL_PARAM_DIF_DL:
						rmon_regist(RMON_PRM_SW_DIF_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
						remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					case CTRL_PROG_SW:
						rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
						remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
						break;
					default:
						break;
					}
				}
				break;
		case	R_RESET_WAIT:
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				if (remotedl_reset_start_retry_check() < 0) {
					// �����̃��[�g�̓��g���C�Ȃ��������肦�Ȃ�
					// ���g���C�I�[�o�[
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					rmon_regist(RMON_RESET_START_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_RESET);
					remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_ERR);
				}
				break;
		default:
			break;
	}
	
	return timerset;

}															

//[]----------------------------------------------------------------------[]
///	@brief			�S�o�b�N�A�b�v����
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			ret		:0�F�o�b�N�A�b�v���s 1�F�o�b�N�A�b�v����
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	remotedl_BackUP( void )							
{															
	t_SysMnt_ErrInfo	errinfo;
	uchar i, ret;
	
	ret = 0;
	// �ݒ�؊��̓o�b�N�A�b�v���s��Ȃ�
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//	if (remotedl_connect_type_get() == CTRL_PARAM_DL) {
	if (remotedl_connect_type_get() == CTRL_PARAM_DL || remotedl_connect_type_get() == CTRL_PARAM_DIF_DL ) {
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		return 1;
	}
	wopelg( OPLOG_ALLBACKUP, 0, 0 );			// ���엚��o�^
	for( i=0; i<BACKUP_RETRY_COUNT; i++){
		errinfo.errmsg = _SYSMNT_ERR_NONE;
		sysmnt_Backup(&errinfo);
		if( errinfo.errmsg == _SYSMNT_ERR_NONE ){
			ret = 1;
			break;
		}
		WACDOG;
	}
	return ret;
}															

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�hRism�ؒf��������
//[]----------------------------------------------------------------------[]
///	@param[in]		evt		:�C�x���g���
///	@param[in]		data	:�v�����
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remote_evt_recv( ushort evt, uchar *data )
{

	uchar status = remotedl_status_get();
	
	switch( evt ){
		case	REMOTE_DL_REQ:
			remote_dl_check( data );				// �v���`�F�b�N
			break;

		case	REMOTE_CONNECT_EVT:
			switch( status ){
				case	R_DL_START:
				case 	R_UP_START:
				case 	R_TEST_CONNECT:
					remotedl_status_set( R_DL_EXEC );			// remotedl_task�N��
					break;


				case	R_SW_START:
					ftp_remote_auto_switch();									// �ؑ֏������s
					// �X�V���s�Ń��Z�b�g
					System_reset();								// Main CPU reset (It will not retrun from this function)
					break;
					
				default:
					break;
			}
			break;
		case	REMOTE_DL_END:
		case	REMOTE_RETRY_SND:
				AutoDL_UpdateTimeCheck();
				break;

		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�v���O�����X�V��������/���s
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	AutoDL_UpdateTimeCheck( void )
{
	ulong	GetDate,NowDate;
	uchar	i,res,status;
	t_prog_chg_info *dl = remotedl_info_get();
	ushort	sec_cnt;
	ulong	code, count;
	MsgBuf	*msb, msg;
	t_TARGET_MSGGET_PRM	WaitMsgID;
	union {
		unsigned short comd;
		unsigned char cc[2];
	} sc;
	
	status = remotedl_status_get();
	
	switch( status ){
		case	R_DL_IDLE:
			// ���g���C�`�F�b�N
			switch (remotedl_connect_type_get()) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			case CTRL_PROG_DL:
//			case CTRL_PARAM_DL:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//			case CTRL_PARAM_DIF_DL:
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)			
//				// �ڑ����g���C�H
//				if (now_retry_active_chk(RETRY_KIND_CONNECT) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT].time, R_DL_START)) {
//					dl->retry[RETRY_KIND_CONNECT].status |= RETRY_EXEC;
//				}
//				// �_�E�����[�h���g���C�H
//				else if (now_retry_active_chk(RETRY_KIND_DL) && dl_start_chk(&dl->retry[RETRY_KIND_DL].time, R_DL_START)) {
//					dl->retry[RETRY_KIND_DL].status |= RETRY_EXEC;
//				}
//				break;
//			case CTRL_PARAM_UPLOAD:
//				// �ڑ����g���C�H
//				if (now_retry_active_chk(RETRY_KIND_CONNECT) && up_wait_chk(&dl->retry[RETRY_KIND_CONNECT].time, R_UP_START)) {
//					dl->retry[RETRY_KIND_CONNECT].status |= RETRY_EXEC;
//				}
//				// �A�b�v���[�h���g���C�H
//				else if (now_retry_active_chk(RETRY_KIND_UL) && up_wait_chk(&dl->retry[RETRY_KIND_UL].time, R_UP_START)) {
//					dl->retry[RETRY_KIND_UL].status |= RETRY_EXEC;
//				}
//				break;
			case CTRL_PROG_DL:
				// �ڑ����g���C�H
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PRG) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT_PRG].time, R_DL_START)) {
					dl->retry[RETRY_KIND_CONNECT_PRG].status |= RETRY_EXEC;
				}
				// �_�E�����[�h���g���C�H
				else if (now_retry_active_chk(RETRY_KIND_DL_PRG) && dl_start_chk(&dl->retry[RETRY_KIND_DL_PRG].time, R_DL_START)) {
					dl->retry[RETRY_KIND_DL_PRG].status |= RETRY_EXEC;
				}
				break;
			case CTRL_PARAM_DL:
				// �ڑ����g���C�H
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT_PARAM_DL].time, R_DL_START)) {
					dl->retry[RETRY_KIND_CONNECT_PARAM_DL].status |= RETRY_EXEC;
				}
				// �_�E�����[�h���g���C�H
				else if (now_retry_active_chk(RETRY_KIND_DL_PARAM) && dl_start_chk(&dl->retry[RETRY_KIND_DL_PARAM].time, R_DL_START)) {
					dl->retry[RETRY_KIND_DL_PARAM].status |= RETRY_EXEC;
				}
				break;
			case CTRL_PARAM_DIF_DL:
				// �ڑ����g���C�H
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL_DIFF) && dl_start_chk(&dl->retry[RETRY_KIND_CONNECT_PARAM_DL_DIFF].time, R_DL_START)) {
					dl->retry[RETRY_KIND_CONNECT_PARAM_DL_DIFF].status |= RETRY_EXEC;
				}
				// �_�E�����[�h���g���C�H
				else if (now_retry_active_chk(RETRY_KIND_DL_PARAM_DIFF) && dl_start_chk(&dl->retry[RETRY_KIND_DL_PARAM_DIFF].time, R_DL_START)) {
					dl->retry[RETRY_KIND_DL_PARAM_DIFF].status |= RETRY_EXEC;
				}
				break;
			case CTRL_PARAM_UPLOAD:
				// �ڑ����g���C�H
				if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_UP) && up_wait_chk(&dl->retry[RETRY_KIND_CONNECT_PARAM_UP].time, R_UP_START)) {
					dl->retry[RETRY_KIND_CONNECT_PARAM_UP].status |= RETRY_EXEC;
				}
				// �A�b�v���[�h���g���C�H
				else if (now_retry_active_chk(RETRY_KIND_UL) && up_wait_chk(&dl->retry[RETRY_KIND_UL].time, R_UP_START)) {
					dl->retry[RETRY_KIND_UL].status |= RETRY_EXEC;
				}
				break;
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			default:
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				return;
				// �O�̂��ߎ��Ɏ��s�����v�����Z�b�g
				remotedl_arrange_next_request();
				break;
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
			}
			break;

		case	R_DL_REQ_RCV:
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				i = PROG_DL_TIME;
				break;
			case CTRL_PARAM_DL:
				i = PARAM_DL_TIME;
				break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			case CTRL_PARAM_DIF_DL:
				i = PARAM_DL_DIF_TIME;
				break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			default:
				return;
			}
			dl_start_chk( &dl->dl_info[i].start_time, R_DL_START );
			break;

		case	R_SW_WAIT:
			res = 0;
		
			for( i=0; i<SW_MAX; i++ ){
				switch (i) {
				case SW_PROG:
				case SW_PROG_EX:
					code = RMON_PRG_SW_START_OK;
					break;
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//				case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
////					code = RMON_PRM_SW_START_OK;
//					if ( remotedl_connect_type_get() ==  CTRL_PARAM_DL ){
//						code = RMON_PRM_SW_START_OK;
//					} else {
//						code = RMON_PRM_SW_DIF_START_OK;
//					}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case SW_PARAM:
					code = RMON_PRM_SW_START_OK;
					break;
				case SW_PARAM_DIFF:
					code = RMON_PRM_SW_DIF_START_OK;
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
					break;
				}
				if( dl->sw_info[i].exec ){
					GetDate = Nrm_YMDHM( &dl->sw_info[i].sw_time );			// ���s���ԃm�[�}���C�Y
					NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// ���ݎ����m�[�}���C�Y
					if( GetDate <= NowDate ){ 								// ��v�������͐ؑ֎������o�߂��Ă���
// GG129000(S) R.Endo 2023/01/26 �Ԕԃ`�P�b�g���X4.1 #6816 ���u�ݒ�_�E�����[�h�̑������s�Ŏ��s(��莞�Ԍo��)��ɍċN����AMANO��ʂ���i�܂� [���ʉ��P���� No 1548]
						// �J�n��������莞�Ԍo�߂��Ă��Ȃ����H
						if ( remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME ) {
							remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
							switch ( remotedl_connect_type_get() ) {
							case CTRL_PROG_DL:
								rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_VER_UP);
								remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
								break;
							case CTRL_PARAM_DL:
								rmon_regist(RMON_PRM_SW_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_PARAM_CHG);
								remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
								break;
							case CTRL_PARAM_DIF_DL:
								rmon_regist(RMON_PRM_SW_DIF_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
								remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
								break;
							case CTRL_PROG_SW:
								rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
								remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
								remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
								break;
							default:
								break;
							}
							continue;
						}
// GG129000(E) R.Endo 2023/01/26 �Ԕԃ`�P�b�g���X4.1 #6816 ���u�ݒ�_�E�����[�h�̑������s�Ŏ��s(��莞�Ԍo��)��ɍċN����AMANO��ʂ���i�܂� [���ʉ��P���� No 1548]
						do {
							// �ҋ@���x�ƈȊO�̏ꍇ
							if (!(OPECTL.Mnt_mod == 0 &&
								(OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100))) {
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_STATUS_NG;
									break;
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//								case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
////									code = RMON_PRM_SW_START_STATUS_NG;
//									if( remotedl_connect_type_get() == CTRL_PARAM_DL ){		// ���u�_�E�����[�h�ڑ���ʎ擾
//										code = RMON_PRM_SW_START_STATUS_NG;			// ���NG
//									} else {
//										code = RMON_PRM_SW_DIF_START_STATUS_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
								case SW_PARAM:
									code = RMON_PRM_SW_START_STATUS_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_STATUS_NG;
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
									break;
								}
								break;
							}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//							// Edy�����Z���^�[�ʐM���A����T���v���{��
//							if ((auto_cnt_prn == 2) ||
//								(auto_syu_prn == 1) || (auto_syu_prn == 2)) {
							// ����T���v���{��
							if ((auto_syu_prn == 1) || (auto_syu_prn == 2)) {
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_COMM_NG;
									break;
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//								case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
////									code = RMON_PRM_SW_START_COMM_NG;
//									if (remotedl_connect_type_get() == CTRL_PARAM_DL ){
//										code = RMON_PRM_SW_START_COMM_NG;
//									} else {
//										code = RMON_PRM_SW_DIF_START_COMM_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
								case SW_PARAM:
									code = RMON_PRM_SW_START_COMM_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_COMM_NG;
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
									break;
								}
								break;
							}
							// �t���b�v���쒆
							if (flap_move_chk()) {
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_FLAP_NG;
									break;
								case SW_PARAM:
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
////									code = RMON_PRM_SW_START_FLAP_NG;
//									if (remotedl_connect_type_get() == CTRL_PARAM_DL ){
//										code = RMON_PRM_SW_START_FLAP_NG;
//									} else {
//										code = RMON_PRM_SW_DIF_START_FLAP_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
									code = RMON_PRM_SW_START_FLAP_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_FLAP_NG;
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
									break;
								}
								break;
							}
							// �h�A���J���Ă��邩�A�\���m�C�h���b�N���������
							if (DOOR_OPEN_OR_SOLENOID_UNROCK()) {
								switch (i) {
								case SW_PROG:
								case SW_PROG_EX:
									code = RMON_PRG_SW_START_DOOR_OPEN_NG;
									break;
								case SW_PARAM:
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
////									code = RMON_PRM_SW_START_DOOR_OPEN_NG;
//									if (remotedl_connect_type_get() == CTRL_PARAM_DL ){
//										code = RMON_PRM_SW_START_DOOR_OPEN_NG;
//									} else {
//										code = RMON_PRM_SW_DIF_START_DOOR_OPEN_NG;
//									}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
									code = RMON_PRM_SW_START_DOOR_OPEN_NG;
									break;
								case SW_PARAM_DIFF:
									code = RMON_PRM_SW_DIF_START_DOOR_OPEN_NG;
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
									break;
								}
								break;
							}
// GG129000(S) R.Endo 2023/01/26 �Ԕԃ`�P�b�g���X4.1 #6816 ���u�ݒ�_�E�����[�h�̑������s�Ŏ��s(��莞�Ԍo��)��ɍċN����AMANO��ʂ���i�܂� [���ʉ��P���� No 1548]
// 							// �J�n��������莞�Ԍo�߂��Ă��Ȃ����H
// // GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
// //							if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
// 							if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// // GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
// // GG120600(S) // Phase9 ��ʖ����`�F�b�N
// //								remotedl_status_set(R_DL_IDLE);
// // GG120600(E) // Phase9 ��ʖ����`�F�b�N
// // GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
// //								remotedl_start_retry_clear();
// 								remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// // GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// 								switch (remotedl_connect_type_get()) {
// 								case CTRL_PROG_DL:
// // GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
// //									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
// 									rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// 									remotedl_complete_request(REQ_KIND_VER_UP);
// 									remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
// 									break;
// 								case CTRL_PARAM_DL:
// // GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
// //									rmon_regist(RMON_PRM_SW_START_RETRY_OVER);
// 									rmon_regist(RMON_PRM_SW_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// 									remotedl_complete_request(REQ_KIND_PARAM_CHG);
// 									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
// 									break;
// // MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
// 								case CTRL_PARAM_DIF_DL:
// // GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
// //									rmon_regist(RMON_PRM_SW_DIF_START_RETRY_OVER);
// 									rmon_regist(RMON_PRM_SW_DIF_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// 									remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// 									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
// 									break;
// // MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
// 								case CTRL_PROG_SW:
// // GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
// //									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
// 									rmon_regist(RMON_PRG_SW_START_OVER_ELAPSED_TIME);
// // GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
// 									remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
// 									remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
// 									break;
// 								default:
// 									break;
// 								}
// 								continue;
// 							}
// GG129000(E) R.Endo 2023/01/26 �Ԕԃ`�P�b�g���X4.1 #6816 ���u�ݒ�_�E�����[�h�̑������s�Ŏ��s(��莞�Ԍo��)��ɍċN����AMANO��ʂ���i�܂� [���ʉ��P���� No 1548]
						} while (0);

						// �X�V�J�nNG
// MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//						if ((code != RMON_PRG_SW_START_OK) &&
//							(code != RMON_PRM_SW_START_OK)) {
						if ((code != RMON_PRG_SW_START_OK) &&
							(code != RMON_PRM_SW_START_OK) && (code != RMON_PRM_SW_DIF_START_OK)) {
// MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
							// �G���[�̊Ď��f�[�^��o�^
							rmon_regist(code);
							if (remotedl_sw_start_retry_check() < 0) {
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//								remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//								remotedl_start_retry_clear();
								remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
								switch (remotedl_connect_type_get()) {
								case CTRL_PROG_DL:
									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_VER_UP);
									remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
									break;
								case CTRL_PARAM_DL:
									rmon_regist(RMON_PRM_SW_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_PARAM_CHG);
									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_TIME, EXEC_STS_ERR);
									break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
								case CTRL_PARAM_DIF_DL:
									rmon_regist(RMON_PRM_SW_DIF_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
									remotedl_comp_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
									break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
								case CTRL_PROG_SW:
									rmon_regist(RMON_PRG_SW_START_RETRY_OVER);
									remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
									remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
									break;
								default:
									break;
								}
							}
							continue;
						}
						// �X�V�J�nOK
						else {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//							remotedl_start_retry_clear();
							remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

							switch (i) {
							case SW_PROG:
							case SW_PROG_EX:
								rmon_regist(RMON_PRG_SW_START_OK);
								break;
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//							case SW_PARAM:
//// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
////								rmon_regist(RMON_PRM_SW_START_OK);
//								if( remotedl_connect_type_get() == (CTRL_PARAM_DL)){
//									rmon_regist(RMON_PRM_SW_START_OK);
//								} else {
//									rmon_regist(RMON_PRM_SW_DIF_START_OK);
//								}
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
							case SW_PARAM:
								rmon_regist(RMON_PRM_SW_START_OK);
								break;
							case SW_PARAM_DIFF:
								rmon_regist(RMON_PRM_SW_DIF_START_OK);
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
								break;
							}
						}
						dl->sw_info[i].exec = 2;							// �ؑ֎��s�t���O�Z�b�g
						res = 1;											// �ؑւ���
// GG120600(S) // Phase9 #5031 �o�[�W�����A�b�v�v���Ɛݒ�ύX�v���������J�n�����A�X�V����(�J�n���X�V)�ŗ\�񂳂�Ă���ꍇ�A�o�[�W�����A�b�v�v���̏��������܂Őݒ�ύX�v���̏��������s���Ȃ�
						// 1�݂�����break
						break;
// GG120600(E) // Phase9 #5031 �o�[�W�����A�b�v�v���Ɛݒ�ύX�v���������J�n�����A�X�V����(�J�n���X�V)�ŗ\�񂳂�Ă���ꍇ�A�o�[�W�����A�b�v�v���̏��������܂Őݒ�ύX�v���̏��������s���Ȃ�
					}
// MH322915(S) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
					// �܂����ԂɂȂ��Ă��Ȃ��H
					else {
						// 5���O�H
						if( UNSENT_TIME_BEFORE_SW == (GetDate - NowDate) ){
							// �p�����[�^�̐؊�������Ȃ��H
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//							if( remotedl_connect_type_get() != CTRL_PARAM_DL ){
							if( remotedl_connect_type_get() != CTRL_PARAM_DL || remotedl_connect_type_get() != CTRL_PARAM_DIF_DL){
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
								// �f�[�^�đ��҂��^�C�}�[�L�����Z��
								RAU_Cancel_RetryTimer();
								// �����M�f�[�^���M
				 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
									NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL, 0);
								}
								else {
									NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL_PHASE1, 0);
								}
							}
						}
					}
// MH322915(E) K.Onodera 2017/05/18 ���u�_�E�����[�h�C��
				}
			}
			
			if( res ){														// �ؑւ�����
				remotedl_exec_info_set( FLASH_WRITE_BEFORE );				// �ڍ׏������Z�b�g�i���Z�b�g�ɂ�鏈�����~�j
				remotedl_status_set( R_SW_START );							// �v���O�����X�V�J�n
				
				dispclr();		

				if( opncls() == 1 ){	// �c�ƒ�
					grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[6]);	/* "�@�@�@�����e�i���X���ł��@�@�@" */
					grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[7]);	/* "�@�@�@�������Z�ł��܂���@�@�@" */
					grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[7]);	/* "     ���΂炭���҂�������     " */
				} else {				// �x�ƒ�
					grachr(2, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR3[6]);	/* "�@�@�@�����e�i���X���ł��@�@�@" */
					grachr(4, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR3[7]);	/* "�@�@�@�������Z�ł��܂���@�@�@" */
					grachr(5, 0, 30, 0, COLOR_WHITE, LCD_BLINK_OFF, OPE_CHR[7]);	/* "     ���΂炭���҂�������     " */
				}
				LcdBackLightCtrl( ON );										// back light ON

				// �ݒ�؊��͂����Ŕ�����
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//				if (remotedl_connect_type_get() == CTRL_PARAM_DL) {
				if (remotedl_connect_type_get() == CTRL_PARAM_DL || remotedl_connect_type_get() == CTRL_PARAM_DIF_DL) {
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
					break;
				}

				// ��������~
				RAU_StopDownline();
				// lkcomtask���~�߂�
				LKcom_f_TaskStop = 1;

				// �������M�ꎞ��~
				ntautoStopReq();
// MH322915(S) K.Onodera 2017/05/22 ���u�_�E�����[�h�C��
				// �f�[�^�đ��҂��^�C�}�[�L�����Z��
				RAU_Cancel_RetryTimer();
// MH322915(E) K.Onodera 2017/05/22 ���u�_�E�����[�h�C��

				// �����M�f�[�^���M
 				if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) {
					NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL, 0);
				}
				else {
					NTNET_Snd_DataTblCtrl(61, NTNET_BUFCTRL_REQ_ALL_PHASE1, 0);
				}
				// �����l�Z�b�g
				sec_cnt = 0;
				res = 0;

				// �����M�f�[�^���M������҂��߁A�����Ń��[�v���񂷁B
				// �K�v�ȃC�x���g�����𔲂��o���悤�ɂ���ׁA���̃C�x���g�͕ێ����ꂽ�܂܂Ƃ���
				memset( &WaitMsgID, 0, sizeof( WaitMsgID ));
				WaitMsgID.Count = 3;
				WaitMsgID.Command[0] = IBK_EVT;
				WaitMsgID.Command[1] = CLOCK_CHG;
				WaitMsgID.Command[2] = TIMEOUT;

				while( 1 ){
					taskchg( IDLETSKNO );									// Change task to idletask
					msb = Target_MsgGet_Range( OPETCBNO, &WaitMsgID );		// ���҂���Ұق�����M�i���͗��߂��܂܁j
					if( NULL == msb ){										// ���҂���ҰقȂ��i����M�j
						continue;
					}
					//	��Mү���ނ���
					memcpy( &msg , msb , sizeof(MsgBuf) );					// ��Mү���ފi�[
					FreeBuf( msb );											// ��Mү�����ޯ̧�J��
					sc.comd = msg.msg.command;

					switch( sc.cc[0] ){
					case MSGGETHIGH(CLOCK_CHG):								// �����i�C�x���g
						// �����ł�CLOCK_CHG�C�x���g�����͍Œ���̏����������Ȃ��B
						// SUBCPU_MONIT�͂����ŃN���A���Ȃ��ƁAE0008����������̂ŃN���A����
						ClkrecUpdate( &msg.msg.data[0] );
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						//LCD�ޯ�ײē��춳�čX�V
//						if(SD_LCD){
//							inc_dct(LCD_LIGTH_CNT,1);
//						}
						// QR���[�_�[���쎞��
						if( QR_READER_USE ){
							inc_dct(QR_READER_CNT,1);
						}
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
						SUBCPU_MONIT = 0;									// ���CPU�Ď��׸޸ر
						err_chk( ERRMDL_MAIN, ERR_MAIN_SUBCPU, 0, 0, 0 );	// �װ����
						break;
					case MSGGETHIGH(IBK_EVT):								// IBK event
						if(( sc.comd == IBK_CTRL_OK ) || ( sc.comd == IBK_CTRL_NG )){
							// 1s�^�C�}�N��
							Lagtim( OPETCBNO, 31, 50 );
						}
						break;
					case MSGGETHIGH(TIMEOUT):
						if (sc.comd == TIMEOUT31) {
							// 1s���ɖ����M�f�[�^�̌������`�F�b�N����
							count = remotedl_unsent_data_count_get();
							// �����M�f�[�^��0�ɂȂ�܂ōX�V�������s��Ȃ�
							if (!count) {
								queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
								res = 1;
							}
							// ������̑��M�V�[�P���X�L�����Z���^�C�}(300s)�o�߂��Ă�
							// �����M�f�[�^��0���ɂȂ�Ȃ��ꍇ�͍X�V�������s�Ƃ���
							else if (++sec_cnt >= UNSENT_DATA_RETRY_COUNT) {
								rmon_regist(RMON_PRG_SW_END_UNSENT_DATA_NG);
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//								remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//								remotedl_start_retry_clear();
								remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
								switch (remotedl_connect_type_get()) {
								case CTRL_PROG_DL:
									remotedl_complete_request(REQ_KIND_VER_UP);
									remotedl_comp_set(INFO_KIND_SW, PROG_DL_TIME, EXEC_STS_ERR);
									break;
								case CTRL_PROG_SW:
									remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
									remotedl_comp_set(INFO_KIND_SW, PROG_ONLY_TIME, EXEC_STS_ERR);
									break;
								default:
									break;
								}
								// �����M�f�[�^�̑��M���s�̂��߃��Z�b�g
								System_reset();
							}
							else {
								// 1s�^�C�}�N��
				 				Lagtim( OPETCBNO, 31, 50 );
							}
						}
						break;
					default:
						break;
					}
					if( res ){
						break;
					}
				}
			}
			break;
		case R_UP_WAIT:					// �A�b�v���[�h�҂�
			up_wait_chk( &dl->dl_info[PARAM_UP_TIME].start_time, R_UP_START );
			break;
		case R_RESET_WAIT:				// ���Z�b�g�҂�
			reset_start_chk( &dl->dl_info[RESET_TIME].start_time, R_RESET_START );
			break;
		default:
			break;
	}
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
	// �����e�ŕύX�t���O���̂����Ă��邩�ꉞ�`�F�b�N�i�ݒ�ύX��Ƀ����e���ʂ����ɓd��OFFON�j
	if(mnt_GetRemoteFtpFlag() != PRM_CHG_REQ_NONE && OPECTL.Mnt_mod == 0){
		// parkingWeb�ڑ�����
		if(_is_ntnet_remote()) {
			// �[���Őݒ�p�����[�^�ύX
			rmon_regist_ex_FromTerminal(RMON_PRM_SW_END_OK_MNT);
			// �[�����Őݒ肪�X�V���ꂽ���߁A�\�񂪓����Ă�����L�����Z�����s��
			remotedl_cancel_setting();
		}
		mnt_SetFtpFlag(PRM_CHG_REQ_NONE);
	}
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���s���菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		rcvdata	:�v�����
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remote_dl_check( uchar	*rcvdata )
{
	uchar	err = REQ_ACCEPT;
	t_ProgDlReq *p;
	BOOL bRT = FALSE;

	p = (t_ProgDlReq *)rcvdata;
	// ��t�s�̏ꍇ�A�ǂݎ̂Ă�
	if (remotedl_check_request(p->ReqKind) == 0) {
		// �Ď��f�[�^��o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		return;
	}

	// �������
	switch (p->ProcKind) {
	case PROC_KIND_EXEC_RT:					// �������s
		bRT = TRUE;
		// no break
	case PROC_KIND_EXEC:					// �\��o�^
		err = remotedl_proc_resv_exec(p, bRT);
		if (err == REQ_ACCEPT) {
			// �X�N���v�g�t�@�C���Ȃ��Ń_�E�����[�h�E�A�b�v���[�h�����{����
			remotedl_script_typeset( REMOTE_NO_SCRIPT );
		}
		break;
	case PROC_KIND_CANCEL:					// ���
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		err = remotedl_proc_cancel(p->ReqKind, &p->InstNo1, FALSE);
		err = remotedl_proc_cancel(p->ReqKind, &p->InstNo1, FALSE,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
//	case PROC_KIND_CHG_TIME:				// �����ύX
//		err = remotedl_proc_chg_time(p);
//		break;
	case PROC_KIND_RESV_INFO:				// �v���m�F
		err = remotedl_proc_resv_info(p);
		break;
	default:
		err = REQ_NOT_ACCEPT;
		// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_PROC_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	}

	if (err == REQ_ACCEPT && p->ReqKind == FTP_TEST_REQ) {
		// �ڑ��e�X�g�́A�������s
		queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
//		memcpy(&g_bk_ProgDlReq, p, sizeof(t_ProgDlReq));
//		// �����̓e�X�g�ڑ���ɕԐM
		return;
	}

//	p->ReqResult = err;
//	NTNET_Snd_Data118_DL(p);

	// �������s�̏ꍇ�A�C�x���g���M���ĊJ�n������s��
	if (p->ProcKind == PROC_KIND_EXEC_RT) {
		queset( OPETCBNO, REMOTE_DL_END, 0, NULL );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�\��o�^
//[]----------------------------------------------------------------------[]
///	@param[in]		*p	: ���u�����e�i���X�v���d��
///	@param[in]		bRT	: �������s�t���O
///	@return			REQ_NOT_ACCEPT = ���s
///					REQ_ACCEPT = ����
///					REQ_PROGNO_ERR = �v���O�������Ԉُ�
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_proc_resv_exec(t_ProgDlReq *p, BOOL bRT)
{
	uchar err = REQ_ACCEPT;
	t_FtpInfo *pFtpInfo;
	int i,nType,len;
	date_time_rec	timeTemp;
	uchar			scriptTemp[12];
	u_ipaddr		ftpaddrTemp;
	ulong			offset = 0;
// MH810103(s) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��
//// MH322915(S) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
//	eVER_TYPE		now_type, req_type;
//// MH322915(E) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
// MH810103(e) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
	u_inst_no*		pinstTemp = remotedl_instNo_get();
	u_inst_no		instTemp;
	uchar			instfrom = remotedl_instFrom_get();

	// ���̎��_�ŕێ����Ă���
	memcpy(&instTemp,pinstTemp,sizeof(u_inst_no));
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

	// �������s�ł����offset���Z�b�g
	if (bRT) {
		offset = RMON_OFFSET_REAL_TIME;
	}

	// �v�����
	switch( p->ReqKind ){
	case VER_UP_REQ:																	// �o�[�W�����A�b�v�v��
	case PROGNO_CHG_REQ:																// ���ԕύX�v��
		if( NOT_DOWNLOADING() ){
			
			do {
				// ***  �v���`�F�b�N�J�n **** //
				// �_�E�����[�h�J�n����
				// �������s���̓`�F�b�N���Ȃ�
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
						err = REQ_NOT_ACCEPT;								// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// �X�V����
				timeTemp.Year = (ushort)(p->SW_Year+2000);
				memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
				if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
					if( timeTemp.Year != 0 && timeTemp.Mon != 0 ){		// 0�N0���͌��ݓ����Ƃ��Ĉ���
						err = REQ_NOT_ACCEPT;							// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// �v���O��������
				memset(scriptTemp, 0, sizeof(scriptTemp));
				memcpy(scriptTemp, p->Prog_No, sizeof(p->Prog_No));
				len = strlen((char *)scriptTemp);
				memcpy(&scriptTemp[len], p->Prog_Ver, sizeof(p->Prog_Ver));

// MH810103(s) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��
//				if (p->ReqKind == VER_UP_REQ) {
//// MH322915(S) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
////					// �u�v���O�������ԁv�̃`�F�b�N
////					if( strncmp((char*)scriptTemp, (char*)VERSNO.ver_part, len) != 0 ){
////						err = REQ_PROGNO_ERR;											// �u�v�����C���v���O�������Ԉُ�v
////						break;
////					}
////					// �u�v���O�����o�[�W�����v�̃`�F�b�N
////					if( strncmp((char*)&scriptTemp[len], (char*)VERSNO.ver_romn, 2) < 0 ){
////						err = REQ_PROGNO_ERR;
////						break;
////					}
//					req_type = GetVersionPrefix( scriptTemp, sizeof(scriptTemp) );						// �v�����ԃv���t�B�b�N�X
//					now_type = GetVersionPrefix( (uchar*)VERSNO.ver_part, sizeof(VERSNO.ver_part) );	// ���ݕ��ԃv���t�B�b�N�X
//					if( req_type == now_type ){
//						if( req_type == VER_TYPE_GW || req_type == VER_TYPE_GG ){
//							// �u�v���O�������ԁv�̃`�F�b�N(���ԓ����H) = GG�Ȃǂ̏ꍇ�A���Ԃ��Ⴄ�͎̂��s
//							if( strncmp( (char*)scriptTemp, (char*)VERSNO.ver_part, len) == 0 ){
//								// �u�v���O�����o�[�W�����v�̃`�F�b�N(�Â��H)
//								if( strncmp( (char*)&scriptTemp[len], (char*)VERSNO.ver_romn, 2) < 0 ){
//									err = REQ_PROGNO_ERR;
//									break;
//								}
//							}
//						}else{
//							// �u�v���O�������ԁv�̃`�F�b�N
//							if( strncmp( (char*)scriptTemp, (char*)VERSNO.ver_part, len) != 0 ){
//								err = REQ_PROGNO_ERR;											// �u�v�����C���v���O�������Ԉُ�v
//								break;
//							}
//							// �u�v���O�����o�[�W�����v�̃`�F�b�N
//							if( strncmp( (char*)&scriptTemp[len], (char*)VERSNO.ver_romn, 2) < 0 ){
//								err = REQ_PROGNO_ERR;
//								break;
//							}
//						}
//					}else{
//						// ����A�ʕ��ԁ˕W���A�W������ʓ��͋֎~�Ƃ���
//						if( now_type == VER_TYPE_GM || req_type == VER_TYPE_GM ){
//							err = REQ_PROGNO_ERR;											// �u�v�����C���v���O�������Ԉُ�v
//						}
//						// �z��O�̕��Ԃɂ͕ύX�ł��Ȃ��悤�ɂ��Ă���
//						if( req_type == VER_TYPE_MAX ){
//							err = REQ_PROGNO_ERR;											// �u�v�����C���v���O�������Ԉُ�v
//						}
//					}
//// MH322915(E) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
//				}
// MH810103(e) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��
				// IP�A�h���X
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ���[�U
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// �p�X���[�h
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  �v���`�F�b�N�I�� **** //

				remotedl_recv_info_clear(PROG_DL_TIME);
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
////				// @todo phase2�ō폜���邱��!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PROG_DL_TIME);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_info_clear(PROG_DL_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

				// ���ߔԍ�
				memcpy(&chg_info.dl_info[PROG_DL_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
				// �v����
				chg_info.dl_info[PROG_DL_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

				// �_�E�����[�h�J�n������ݒ�
				if (bRT) {
					// �������s�͌��ݎ������Z�b�g
					memcpy( &chg_info.dl_info[PROG_DL_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.dl_info[PROG_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
					memcpy( &chg_info.dl_info[PROG_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );
				}

				// �X�V������ݒ�
				if (p->SW_Year == 0 && p->SW_Mon == 0) {
					// 0�N0���͌��ݎ����Ƃ��Ĉ���
					memcpy( &chg_info.sw_info[SW_PROG].sw_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.sw_info[SW_PROG].sw_time.Year = (ushort)(p->SW_Year+2000);
					memcpy( &chg_info.sw_info[SW_PROG].sw_time.Mon, &p->SW_Mon, 4 );
				}

				// �v���O��������
				memcpy(&chg_info.script[PROGNO_KIND_DL], scriptTemp, sizeof(scriptTemp));

				// IP�A�h���X
				chg_info.dl_info[PROG_DL_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTP�|�[�g�ԍ�
				chg_info.dl_info[PROG_DL_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ���[�U
				strncpy((char *)chg_info.dl_info[PROG_DL_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// �p�X���[�h
				strncpy((char *)chg_info.dl_info[PROG_DL_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PRG);
				retry_info_clr(RETRY_KIND_DL_PRG);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PROG_DL_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				remotedl_accept_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				remotedl_connect_type_set( CTRL_PROG_DL );
//				remotedl_status_set( R_DL_REQ_RCV );									// �J�n�\���ԂƂ��ă_�E�����[�h�J�n�����ɂȂ�܂ł܂�
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_set(INFO_KIND_START, PROG_DL_TIME, &chg_info.dl_info[PROG_DL_TIME].start_time);
				remotedl_time_set(INFO_KIND_SW, PROG_DL_TIME, &chg_info.sw_info[SW_PROG].sw_time);

				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRG_DL_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRG_DL_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				remotedl_monitor_info_set(0);
			}
			else {
				// �Ď��f�[�^�o�^
				if (err == REQ_NOT_ACCEPT) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//					rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				}
				else {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//					rmon_regist_ex(RMON_PRG_DL_REQ_PRGNO_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_DL_REQ_PRGNO_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				}
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	case PARAM_CHG_REQ:																	// �ݒ�ύX�v��
		if( NOT_DOWNLOADING() ){
			do {
				// ***  �v���`�F�b�N�J�n **** //
				// �_�E�����[�h�J�n����
				// �������s���̓`�F�b�N���Ȃ�
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
						err = REQ_NOT_ACCEPT;								// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// �X�V����
				timeTemp.Year = (ushort)(p->SW_Year+2000);
				memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
				if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
					if( timeTemp.Year != 0 && timeTemp.Mon != 0 ){		// 0�N0���͌��ݓ����Ƃ��Ĉ���
						err = REQ_NOT_ACCEPT;							// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// IP�A�h���X
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ���[�U
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// �p�X���[�h
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  �v���`�F�b�N�I�� **** //
				
				remotedl_recv_info_clear(PARAM_DL_TIME);
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
////				// @todo phase2�ō폜���邱��!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PARAM_DL_TIME);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_info_clear(PARAM_DL_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

				// ���ߔԍ�
				memcpy(&chg_info.dl_info[PARAM_DL_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
				// �v����
				chg_info.dl_info[PARAM_DL_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

				// �_�E�����[�h�J�n������ݒ�
				if (bRT) {
					// �������s�͌��ݎ������Z�b�g
					memcpy( &chg_info.dl_info[PARAM_DL_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.dl_info[PARAM_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
					memcpy( &chg_info.dl_info[PARAM_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );
				}

				// �X�V������ݒ�
				if (p->SW_Year == 0 && p->SW_Mon == 0) {
					// 0�N0���͌��ݎ����Ƃ��Ĉ���
					memcpy( &chg_info.sw_info[SW_PARAM].sw_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
					memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );
				}

				// IP�A�h���X
				chg_info.dl_info[PARAM_DL_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTP�|�[�g�ԍ�
				chg_info.dl_info[PARAM_DL_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ���[�U
				strncpy((char *)chg_info.dl_info[PARAM_DL_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// �p�X���[�h
				strncpy((char *)chg_info.dl_info[PARAM_DL_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
				retry_info_clr(RETRY_KIND_DL_PARAM);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_DL_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//				remotedl_accept_request(REQ_KIND_PARAM_CHG);
//				remotedl_connect_type_set( CTRL_PARAM_DL );
				if( p->ReqKind == PARAM_CHG_REQ ){
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					remotedl_connect_type_set( CTRL_PARAM_DL );		// ��ʃZ�b�g
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					remotedl_accept_request(REQ_KIND_PARAM_CHG);	// �ݒ�ύX�v��
				}else{
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					remotedl_connect_type_set( CTRL_PARAM_DIF_DL );
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					remotedl_accept_request(REQ_KIND_PARAM_DIF_CHG);	// �����ݒ�ύX�v��
				}
// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				remotedl_status_set( R_DL_REQ_RCV );									// �J�n�\���ԂƂ��ă_�E�����[�h�J�n�����ɂȂ�܂ł܂�
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_set(INFO_KIND_START, PARAM_DL_TIME, &chg_info.dl_info[PARAM_DL_TIME].start_time);
				remotedl_time_set(INFO_KIND_SW, PARAM_DL_TIME, &chg_info.sw_info[SW_PARAM].sw_time);

				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRM_DL_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRM_DL_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				remotedl_monitor_info_set(0);
			}
			else {
				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή� #4960
////				rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//				rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή� #4960
				rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή� #4960
////			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//			rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή� #4960
			rmon_regist_ex(RMON_PRM_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case PARAM_DIFF_CHG_REQ:																	// �ݒ�ύX�v��
		if( NOT_DOWNLOADING() ){
			do {
				// ***  �v���`�F�b�N�J�n **** //
				// �_�E�����[�h�J�n����
				// �������s���̓`�F�b�N���Ȃ�
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
						err = REQ_NOT_ACCEPT;								// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// �X�V����
				timeTemp.Year = (ushort)(p->SW_Year+2000);
				memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
				if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
					if( timeTemp.Year != 0 && timeTemp.Mon != 0 ){		// 0�N0���͌��ݓ����Ƃ��Ĉ���
						err = REQ_NOT_ACCEPT;							// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// IP�A�h���X
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ���[�U
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// �p�X���[�h
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  �v���`�F�b�N�I�� **** //
				
				remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
////				// @todo phase2�ō폜���邱��!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PARAM_DL_DIF_TIME);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_info_clear(PARAM_DL_DIF_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

				// ���ߔԍ�
				memcpy(&chg_info.dl_info[PARAM_DL_DIF_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
				// �v����
				chg_info.dl_info[PARAM_DL_DIF_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

				// �_�E�����[�h�J�n������ݒ�
				if (bRT) {
					// �������s�͌��ݎ������Z�b�g
					memcpy( &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
					memcpy( &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Mon, &p->DL_Mon, 4 );
				}

				// �X�V������ݒ�
				if (p->SW_Year == 0 && p->SW_Mon == 0) {
					// 0�N0���͌��ݎ����Ƃ��Ĉ���
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//					memcpy( &chg_info.sw_info[SW_PARAM].sw_time, &CLK_REC, sizeof(date_time_rec) );
					memcpy( &chg_info.sw_info[SW_PARAM_DIFF].sw_time, &CLK_REC, sizeof(date_time_rec) );
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
				}
				else {
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//					chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
//					memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );
					chg_info.sw_info[SW_PARAM_DIFF].sw_time.Year = (ushort)(p->SW_Year+2000);
					memcpy( &chg_info.sw_info[SW_PARAM_DIFF].sw_time.Mon, &p->SW_Mon, 4 );
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
				}

				// IP�A�h���X
				chg_info.dl_info[PARAM_DL_DIF_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTP�|�[�g�ԍ�
				chg_info.dl_info[PARAM_DL_DIF_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ���[�U
				strncpy((char *)chg_info.dl_info[PARAM_DL_DIF_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// �p�X���[�h
				strncpy((char *)chg_info.dl_info[PARAM_DL_DIF_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
				retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				if( p->ReqKind == PARAM_CHG_REQ ){
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					remotedl_connect_type_set( CTRL_PARAM_DL );
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					remotedl_accept_request(REQ_KIND_PARAM_CHG);
				}else{
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					remotedl_connect_type_set( CTRL_PARAM_DIF_DL );
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					remotedl_accept_request(REQ_KIND_PARAM_DIF_CHG);
				}
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				remotedl_status_set( R_DL_REQ_RCV );									// �J�n�\���ԂƂ��ă_�E�����[�h�J�n�����ɂȂ�܂ł܂�
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_set(INFO_KIND_START, PARAM_DL_DIF_TIME, &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time);
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//				remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM].sw_time);
				remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM_DIFF].sw_time);
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���

				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
////				rmon_regist_ex(RMON_PRM_DL_REQ_OK+offset, &p->InstNo1);
//				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_OK+offset, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				remotedl_monitor_info_set(0);
			}
			else {
				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
////				rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
				rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
////			rmon_regist_ex(RMON_PRG_DL_REQ_NG+offset, &p->InstNo1);
//			rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
			rmon_regist_ex(RMON_PRM_DIF_DL_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
// GG120600(E) // MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)

	case PARAM_UPLOAD_REQ:																// �ݒ�v��
// GG124100(S) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
// 		if( NOT_DOWNLOADING() ){														// ���u�_�E�����[�h�����s
		if ( NOT_DOWNLOADING() && (OPECTL.lcd_prm_update == 0) ) {	// ���u�_�E�����[�h�����s �� LCD�ւ̃p�����[�^�A�b�v���[�h���ł͂Ȃ�
// GG124100(E) R.Endo 2022/09/28 �Ԕԃ`�P�b�g���X3.0 #6560 �ċN�������i���O�̃G���[�F���b�Z�[�W�L���[�t�� E0011�j [���ʉ��P���� No1528]
			do {
				// ***  �v���`�F�b�N�J�n **** //
				// �A�b�v���[�h�J�n����
				// �������s���̓`�F�b�N���Ȃ�
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
						err = REQ_NOT_ACCEPT;								// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// IP�A�h���X
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ���[�U
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// �p�X���[�h
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  �v���`�F�b�N�I�� **** //

				remotedl_recv_info_clear(PARAM_UP_TIME);
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
////				// @todo phase2�ō폜���邱��!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PARAM_UP_TIME);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_info_clear(PARAM_UP_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

				// ���ߔԍ�
				memcpy(&chg_info.dl_info[PARAM_UP_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
				// �v����
				chg_info.dl_info[PARAM_UP_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

				// �A�b�v���[�h�J�n������ݒ�
				if (bRT) {
					// �������s�͌��ݎ������Z�b�g
					memcpy( &chg_info.dl_info[PARAM_UP_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					memcpy( &chg_info.dl_info[PARAM_UP_TIME].start_time, &timeTemp, sizeof(timeTemp) );
				}

				// IP�A�h���X
				chg_info.dl_info[PARAM_UP_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTP�|�[�g�ԍ�
				chg_info.dl_info[PARAM_UP_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ���[�U
				strncpy((char *)chg_info.dl_info[PARAM_UP_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// �p�X���[�h
				strncpy((char *)chg_info.dl_info[PARAM_UP_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
				retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
				retry_info_clr(RETRY_KIND_UL);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				remotedl_accept_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				remotedl_connect_type_set( CTRL_PARAM_UPLOAD );
//				remotedl_status_set( R_UP_WAIT );
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_set(INFO_KIND_START, PARAM_UP_TIME, &chg_info.dl_info[PARAM_UP_TIME].start_time);

				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRM_UP_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRM_UP_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				remotedl_monitor_info_set(0);
			}
			else {
				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
		else {
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_PRM_UP_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	case RESET_REQ:
		if( NOT_DOWNLOADING() ){														// ���u�_�E�����[�h�����s
			do{
				// ***  �v���`�F�b�N�J�n **** //
				// ���Z�b�g�J�n����
				// �������s���̓`�F�b�N���Ȃ�
				if (!bRT) {
					timeTemp.Year = (ushort)(p->DL_Year+2000);
					memcpy( &timeTemp.Mon, &p->DL_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
						err = REQ_NOT_ACCEPT;								// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}
				// ***  �v���`�F�b�N�I�� **** //

				remotedl_recv_info_clear(RESET_TIME);
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
////				// @todo phase2�ō폜���邱��!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(RESET_TIME);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_info_clear(RESET_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

				// ���ߔԍ�
				memcpy(&chg_info.dl_info[RESET_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
				// �v����
				chg_info.dl_info[RESET_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

				// ���Z�b�g�J�n������ݒ�
				if (bRT) {
					memcpy( &chg_info.dl_info[RESET_TIME].start_time, &CLK_REC, sizeof(date_time_rec) );
				}
				else {
					memcpy( &chg_info.dl_info[RESET_TIME].start_time, &timeTemp, sizeof(timeTemp) );
				}

			} while (0);

			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				remotedl_accept_request(REQ_KIND_RESET);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				remotedl_connect_type_set( CTRL_RESET );
//				remotedl_status_set( R_RESET_WAIT );										// ���Z�b�g�҂�
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_set(INFO_KIND_START, RESET_TIME, &chg_info.dl_info[RESET_TIME].start_time);

				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_RESET_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_RESET_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				remotedl_monitor_info_set(0);
			}
			else {
				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1);
				rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
		else{
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_RESET_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	case FTP_TEST_REQ:
		// �����e�i���X�����H�_�E�����[�h�����H
		if (OPECTL.Mnt_mod != 0 || DOWNLOADING() ) {
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1);
			rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}else{
			do {
				// ***  �v���`�F�b�N�J�n **** //
				// IP�A�h���X
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ���[�U
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// �p�X���[�h
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  �v���`�F�b�N�I�� **** //
				remotedl_recv_info_clear(TEST_TIME);

				// ���ߔԍ�
				memcpy(&chg_info.dl_info[TEST_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
				// �v����
				chg_info.dl_info[TEST_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

				// IP�A�h���X
				chg_info.dl_info[TEST_TIME].ftpaddr.uladdr = ftpaddrTemp.uladdr;

				// FTP�|�[�g�ԍ�
				chg_info.dl_info[TEST_TIME].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

				// ���[�U
				strncpy((char *)chg_info.dl_info[TEST_TIME].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

				// �p�X���[�h
				strncpy((char *)chg_info.dl_info[TEST_TIME].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
			} while (0);
			if (err == REQ_ACCEPT) {
				remotedl_chg_info_bkup();														// �o�b�N�A�b�v
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				remotedl_accept_request(REQ_KIND_TEST);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				remotedl_connect_type_set( CTRL_CONNECT_CHK );
//				remotedl_status_set( R_TEST_CONNECT );
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_monitor_info_set(0);
			}
			else {
				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1);
				rmon_regist_ex(RMON_FTP_TEST_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
		break;
	case PROG_ONLY_CHG_REQ:					// �v���O�����ؑ֗v��
		if( NOT_DOWNLOADING() ){														// ���u�_�E�����[�h�����s
			do {
				// ***  �v���`�F�b�N�J�n **** //
				// �X�V����
				// �������s���̓`�F�b�N���Ȃ�
				if (!bRT) {
					timeTemp.Year = (ushort)(p->SW_Year+2000);
					memcpy( &timeTemp.Mon, &p->SW_Mon, 4 );
					if( !remote_date_chk(&timeTemp)){						// �N���������̃`�F�b�N
						err = REQ_NOT_ACCEPT;								// �ُ�l�Ȃ̂Ŏ�t�s�ŕԂ�
						break;
					}
				}

				// �v���O�����f�[�^�`�F�b�N
				if (!IsMainProgExist()) {
					err = REQ_SW_PROG_NONE;								// �v���O�����f�[�^���Ȃ�
					break;
				}

				// ���ԃ`�F�b�N
				memset(scriptTemp, 0, sizeof(scriptTemp));
				memcpy(scriptTemp, p->Prog_No, sizeof(p->Prog_No));
				len = strlen((char *)scriptTemp);
				memcpy(&scriptTemp[len], p->Prog_Ver, sizeof(p->Prog_Ver));

				if( strncmp((char*)scriptTemp, (char*)ver_datwk_prog, len+sizeof(p->Prog_Ver)) != 0 ){
					err = REQ_SW_PROGNO_ERR;							// ���ԕs��v
					break;
				}
				// ***  �v���`�F�b�N�I�� **** //
				remotedl_recv_info_clear(PROG_ONLY_TIME);
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
////				// @todo phase2�ō폜���邱��!!
////				remotedl_time_info_clear_all();
//				remotedl_time_info_clear(PROG_ONLY_TIME);
//// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_info_clear(PROG_ONLY_TIME,&instTemp,instfrom);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

				// ���ߔԍ�
				memcpy(&chg_info.dl_info[PROG_ONLY_TIME].u, &p->InstNo1, sizeof(u_inst_no));
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
				// �v����
				chg_info.dl_info[PROG_ONLY_TIME].From = p->RmonFrom;
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

				// �X�V����
				if (bRT) {
					memcpy(&chg_info.sw_info[SW_PROG_EX].sw_time, &CLK_REC, sizeof(date_time_rec));
				}
				else {
					memcpy(&chg_info.sw_info[SW_PROG_EX].sw_time, &timeTemp, sizeof(timeTemp));
				}

				// �v���O��������
				memcpy(&chg_info.script[PROGNO_KIND_SW], scriptTemp, sizeof(scriptTemp));

			} while (0);
			if (err == REQ_ACCEPT) {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PROG_ONLY_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				remotedl_accept_request(REQ_KIND_PROG_ONLY_CHG);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				remotedl_connect_type_set( CTRL_PROG_SW );
//				remotedl_status_set( R_SW_WAIT );
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				remotedl_time_set(INFO_KIND_SW, PROG_ONLY_TIME, &chg_info.sw_info[SW_PROG_EX].sw_time);
				// �v���O�����؊��t���O��ON
				chg_info.sw_status[SW_PROG] = 1;
				chg_info.sw_info[SW_PROG_EX].exec = 1;

				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRG_SW_REQ_OK+offset, &p->InstNo1);
				rmon_regist_ex(RMON_PRG_SW_REQ_OK+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
				remotedl_monitor_info_set(0);
			}
			else {
				// �Ď��f�[�^�o�^
				switch (err) {
				case REQ_NOT_ACCEPT:
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//					rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
					break;
				case REQ_SW_PROG_NONE:
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//					rmon_regist_ex(RMON_PRG_SW_REQ_PRG_NONE_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_SW_REQ_PRG_NONE_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
					break;
				case REQ_SW_PROGNO_ERR:
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//					rmon_regist_ex(RMON_PRG_SW_REQ_PRGNO_NG+offset, &p->InstNo1);
					rmon_regist_ex(RMON_PRG_SW_REQ_PRGNO_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
					break;
				}
			}
		}else{
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_SW_REQ_NG+offset, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	case FTP_CHG_REQ:					// FTP�ݒ�ύX�v��
		if( NOT_DOWNLOADING() ){														// ���u�_�E�����[�h�����s
			do {
				// ***  �v���`�F�b�N�J�n **** //
				// IP�A�h���X
				pFtpInfo = remotedl_ftp_info_get();
				for (i = 0; i < 4; i++) {
					ftpaddrTemp.ucaddr[i] = (uchar)astoin((uchar *)&pFtpInfo->DestIP[i*3], 3);
				}
				if (ftpaddrTemp.uladdr == 0) {
					err = REQ_NOT_ACCEPT;
					break;
				}

				// ���[�U
				if (pFtpInfo->FTPUser[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}
				// �p�X���[�h
				if (pFtpInfo->FTPPasswd[0] == '\0') {
					err = REQ_NOT_ACCEPT;
					break;
				}			
				// ***  �v���`�F�b�N�I�� **** //

				for( nType = 0; nType  < TIME_MAX; nType++) {
					// ���Ƃ��Ƃ����ꂩ����ł���΁A�Z�b�g���Ȃ�
					if( chg_info.dl_info[nType].ftpuser[0] == '\0' || chg_info.dl_info[nType].ftppasswd[0] == '\0'){
						continue;
					}
					// IP�A�h���X
					chg_info.dl_info[nType].ftpaddr.uladdr = ftpaddrTemp.uladdr;

					// FTP�|�[�g�ԍ�
					chg_info.dl_info[nType].ftpport = astoin(pFtpInfo->FTPCtrlPort, sizeof(pFtpInfo->FTPCtrlPort));

					// ���[�U
					strncpy((char *)chg_info.dl_info[nType].ftpuser, (char *)pFtpInfo->FTPUser, sizeof(pFtpInfo->FTPUser));

					// �p�X���[�h
					strncpy((char *)chg_info.dl_info[nType].ftppasswd, (char *)pFtpInfo->FTPPasswd, sizeof(pFtpInfo->FTPPasswd));
				}
			} while (0);

			if (err == REQ_ACCEPT) {
				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_FTP_CHG_OK, &p->InstNo1);
				rmon_regist_ex(RMON_FTP_CHG_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
			else {
				// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1);
				rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}else{
			err = REQ_NOT_ACCEPT;
			// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1);
			rmon_regist_ex(RMON_FTP_CHG_NG, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	default:
		err = REQ_NOT_ACCEPT;
		// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	}
	return err;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�\�񒆎~
//[]----------------------------------------------------------------------[]
///	@param[in]		ReqKind		: �v�����
///	@param[in]		*pInstNo	: ���ߔԍ�
///	@return			REQ_NOT_ACCEPT = ���s
///					REQ_ACCEPT = ����
///					REQ_RCV_NONE = ����M
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt)
uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt,uchar from)
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
{
	uchar err = REQ_RCV_NONE;
// GG120600(S) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
//	uchar connect = remotedl_connect_type_get();
// GG129000(S) R.Endo 2023/01/18 �Ԕԃ`�P�b�g���X4.0 #6804 �����ݒ�_�E�����[�h���̗\�����Ŏ������������ [���ʉ��P���� No1545]
// 	uchar connect = remotedl_request_kind_to_connect_type(remotedl_connect_type_get());
	uchar connect = remotedl_connect_type_get();
// GG129000(E) R.Endo 2023/01/18 �Ԕԃ`�P�b�g���X4.0 #6804 �����ݒ�_�E�����[�h���̗\�����Ŏ������������ [���ʉ��P���� No1545]
// GG120600(E) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
// GG120600(S) // Phase9 ���ʂ���ʂ���
//	uchar ret = remotedl_result_get(RES_DL);
	uchar ret = EXCLUDED;
	switch(connect){
	case CTRL_PROG_DL:					// �v���O�����_�E�����[�h
		ret = remotedl_result_get(RES_DL_PROG);
		break;
	case CTRL_PARAM_DL:					// �ݒ�_�E�����[�h
		ret = remotedl_result_get(RES_DL_PARAM);
		break;
	case CTRL_PARAM_UPLOAD:				// �ݒ�A�b�v���[�h
		ret = remotedl_result_get(RES_UP);
		break;
	case CTRL_PROG_SW:					// �v���O�����؊�
		ret = remotedl_result_get(RES_SW_PROG);
		break;
	case CTRL_PARAM_DIF_DL:				// �����ݒ�_�E�����[�h
		ret = remotedl_result_get(RES_DL_PARAM_DIF);
		break;
	case CTRL_NONE:
	case CTRL_RESET:					// ���Z�b�g
	case CTRL_CONNECT_CHK:				// �ڑ��e�X�g
		break;
	}
// GG120600(E) // Phase9 ���ʂ���ʂ���

	// �v�����
	switch (ReqKind) {
	case VER_UP_REQ:
	case PROGNO_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_VER_UP)) {
			// �o�[�W�����A�b�v�v��������
			if (connect == CTRL_PROG_DL) {
				// �_�E�����[�h���s���H
				// �v���O�����؊����H
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// �v�����
					remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					// �ҋ@���ɑJ��
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// �v�����
				remotedl_complete_request(REQ_KIND_VER_UP);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				// ��ʂ��ăZ�b�g
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				// �ݒ�ύX�v���������̏ꍇ�͌��ʂ��ăZ�b�g
// GG120600(S) // Phase9 ���ʂ���ʂ���
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
////				if (connect == CTRL_PARAM_DL) {
//				if (connect == CTRL_PARAM_DL || connect == CTRL_PARAM_DIF_DL) {
//// GG120600(E) // MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//					chg_info.result[RES_DL] = ret;
//				}
				if (connect == CTRL_PARAM_DL) {
					chg_info.result[RES_DL_PARAM] = ret;
				}else if(connect == CTRL_PARAM_DIF_DL ){
					chg_info.result[RES_DL_PARAM_DIF] = ret;
				}
// GG120600(E) // Phase9 ���ʂ���ʂ���
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// �Ď��f�[�^�o�^
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRG_DL_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRG_DL_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PROG_DL_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
			if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRG_DL_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRG_DL_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
// GG120600(S) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		else if( err == REQ_RCV_NONE){
			// remotedl_is_accept����폜����Ă��邽��
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PROG_DL_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		break;
	case PARAM_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_CHG)) {
			// �ݒ�ύX�v��������
			if (connect == CTRL_PARAM_DL) {
				// �_�E�����[�h���s���H
				// �ݒ�؊����H
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// �v�����
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					// �ҋ@���ɑJ��
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// �v�����
				remotedl_complete_request(REQ_KIND_PARAM_CHG);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				// ��ʂ��ăZ�b�g
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				// �o�[�W�����A�b�v�v���������̏ꍇ�͌��ʂ��ăZ�b�g
				if (connect == CTRL_PROG_DL) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//					chg_info.result[RES_DL] = ret;
					chg_info.result[RES_DL_PROG] = ret;
// GG120600(E) // Phase9 ���ʂ���ʂ���
				}
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// �Ď��f�[�^�o�^
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRM_DL_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRM_DL_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
			if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRM_DL_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRM_DL_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
// GG120600(S) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		else if( err == REQ_RCV_NONE){
			// remotedl_is_accept����폜����Ă��邽��
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		break;
// MH810100(S) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case PARAM_DIFF_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_DIF_CHG)) {	// �v����t�`�F�b�N
			// �ݒ�ύX�v��������
			if ( connect == CTRL_PARAM_DIF_DL ) {
				// �_�E�����[�h���s���H
				// �ݒ�؊����H
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// �v�����
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					// �ҋ@���ɑJ��
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// �v�����
				remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				// ��ʂ��ăZ�b�g
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				// �o�[�W�����A�b�v�v���������̏ꍇ�͌��ʂ��ăZ�b�g
				if (connect == CTRL_PROG_DL) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//					chg_info.result[RES_DL] = ret;
					chg_info.result[RES_DL_PROG] = ret;
// GG120600(E) // Phase9 ���ʂ���ʂ���
				}
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// �Ď��f�[�^�o�^
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
////			rmon_regist_ex(RMON_PRM_DL_CANCEL_OK, pInstNo);
//			rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_OK, pInstNo);
//// MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
			rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
			if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
////				rmon_regist_ex(RMON_PRM_DL_CANCEL_NG, pInstNo);
//				rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_NG, pInstNo);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
				rmon_regist_ex(RMON_PRM_DIF_DL_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
// GG120600(S) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		else if( err == REQ_RCV_NONE){
			// remotedl_is_accept����폜����Ă��邽��
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		break;
// MH810100(E) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case PARAM_UPLOAD_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_UL)) {
			// �ݒ�v��(�A�b�v���[�h)������
			if (connect == CTRL_PARAM_UPLOAD) {
				// �A�b�v���[�h���s���H
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// �v�����
					remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					// �ҋ@���ɑJ��
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// �v�����
				remotedl_complete_request(REQ_KIND_PARAM_UL);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				// ��ʂ��ăZ�b�g
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// �Ď��f�[�^�o�^
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRM_UP_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRM_UP_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
			if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRM_UP_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRM_UP_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
// GG120600(S) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		else if( err == REQ_RCV_NONE){
			// remotedl_is_accept����폜����Ă��邽��
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		break;
	case RESET_REQ:
		if (remotedl_is_accept(REQ_KIND_RESET)) {
			// ���Z�b�g�v��������
			if (connect == CTRL_RESET) {
				// ���Z�b�g���s���H
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// �v�����
					remotedl_complete_request(REQ_KIND_RESET);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					// �ҋ@���ɑJ��
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// �v�����
				remotedl_complete_request(REQ_KIND_RESET);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				// ��ʂ��ăZ�b�g
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// �Ď��f�[�^�o�^
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_RESET_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_RESET_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[RESET_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
			if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_RESET_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_RESET_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
// GG120600(S) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		else if( err == REQ_RCV_NONE){
			// remotedl_is_accept����폜����Ă��邽��
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[RESET_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		break;
	case PROG_ONLY_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PROG_ONLY_CHG)) {
			// �v���O�����؊��v��������
			if (connect == CTRL_PROG_SW) {
				// �v���O�����؊����H
				if (DOWNLOADING()) {
					err = REQ_CANCEL_NOT_ACCEPT;
				}
				else {
					// �v�����
					remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//					// �ҋ@���ɑJ��
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
					err = REQ_CANCEL_ACCEPT;
				}
			}
			else {
				// �v�����
				remotedl_complete_request(REQ_KIND_PROG_ONLY_CHG);
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//				// ��ʂ��ăZ�b�g
//				remotedl_connect_type_set(connect);
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
				err = REQ_CANCEL_ACCEPT;
			}
		}
		// �Ď��f�[�^�o�^
		if (err == REQ_CANCEL_ACCEPT) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRG_SW_CANCEL_OK, pInstNo);
			rmon_regist_ex(RMON_PRG_SW_CANCEL_OK, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PROG_ONLY_TIME], 0, sizeof(t_remote_time_info));
		}
		else if (err == REQ_CANCEL_NOT_ACCEPT) {
			// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
			if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				rmon_regist_ex(RMON_PRG_SW_CANCEL_NG, pInstNo);
				rmon_regist_ex(RMON_PRG_SW_CANCEL_NG, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
			}
		}
// GG120600(S) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		else if( err == REQ_RCV_NONE){
			// remotedl_is_accept����폜����Ă��邽��
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PROG_ONLY_TIME], 0, sizeof(t_remote_time_info));
		}
// GG120600(E) // Phase9 #5070 ���u�����e�i���X�m�F��ʂŎ��s�ς݋y�уG���[�̃X�e�[�^�X�̃f�[�^���N���A����Ȃ�
		break;
	default:
		err = REQ_CANCEL_NOT_ACCEPT;
		// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
		if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	}
	if (err == REQ_RCV_NONE) {
		// �����e�i���X����NG�̊Ď��f�[�^��o�^���Ȃ�
		if (!bMnt) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, pInstNo,from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
	}
	return err;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�����ύX
//[]----------------------------------------------------------------------[]
///	@param[in]		*p	: ���u�����e�i���X�v���d��
///	@return			REQ_NOT_ACCEPT = ���s
///					REQ_ACCEPT = ����
///					REQ_RCV_NONE = ����M
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_proc_chg_time(t_ProgDlReq *p)
{
	uchar err = REQ_RCV_NONE;

	// �v�����
	switch (p->ReqKind) {
	case VER_UP_REQ:
	case PROGNO_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_VER_UP)) {
			// �_�E�����[�h�J�n������ݒ�
			chg_info.dl_info[PROG_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PROG_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );

			// �X�V������ݒ�
			chg_info.sw_info[SW_PROG].sw_time.Year = (ushort)(p->SW_Year+2000);
			memcpy( &chg_info.sw_info[SW_PROG].sw_time.Mon, &p->SW_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, PROG_DL_TIME, &chg_info.dl_info[PROG_DL_TIME].start_time);
			remotedl_time_set(INFO_KIND_SW, PROG_DL_TIME, &chg_info.sw_info[SW_PROG].sw_time);
		}
		break;
	case PARAM_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_CHG)) {
			// �_�E�����[�h�J�n������ݒ�
			chg_info.dl_info[PARAM_DL_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PARAM_DL_TIME].start_time.Mon, &p->DL_Mon, 4 );

			// �X�V������ݒ�
			chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
			memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, PARAM_DL_TIME, &chg_info.dl_info[PARAM_DL_TIME].start_time);
			remotedl_time_set(INFO_KIND_SW, PARAM_DL_TIME, &chg_info.sw_info[SW_PARAM].sw_time);
		}
		break;
// MH810100(S) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case PARAM_DIFF_CHG_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_DIF_CHG)) {
			// �_�E�����[�h�J�n������ݒ�
			chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time.Mon, &p->DL_Mon, 4 );

			// �X�V������ݒ�
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//			chg_info.sw_info[SW_PARAM].sw_time.Year = (ushort)(p->SW_Year+2000);
//			memcpy( &chg_info.sw_info[SW_PARAM].sw_time.Mon, &p->SW_Mon, 4 );
			chg_info.sw_info[SW_PARAM_DIFF].sw_time.Year = (ushort)(p->SW_Year+2000);
			memcpy( &chg_info.sw_info[SW_PARAM_DIFF].sw_time.Mon, &p->SW_Mon, 4 );
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���

			remotedl_time_set(INFO_KIND_START, PARAM_DL_DIF_TIME, &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time);
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//			remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM].sw_time);
			remotedl_time_set(INFO_KIND_SW, PARAM_DL_DIF_TIME, &chg_info.sw_info[SW_PARAM_DIFF].sw_time);
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
		}
		break;
// MH810100(E) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)

	case PARAM_UPLOAD_REQ:
		if (remotedl_is_accept(REQ_KIND_PARAM_UL)) {
			// �J�n������ݒ�
			chg_info.dl_info[PARAM_UP_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[PARAM_UP_TIME].start_time.Mon, &p->DL_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, PARAM_UP_TIME, &chg_info.dl_info[PARAM_UP_TIME].start_time);
		}
		break;
	case RESET_REQ:
		if (remotedl_is_accept(REQ_KIND_RESET)) {
			// �J�n������ݒ�
			chg_info.dl_info[RESET_TIME].start_time.Year = (ushort)(p->DL_Year+2000);
			memcpy( &chg_info.dl_info[RESET_TIME].start_time.Mon, &p->DL_Mon, 4 );

			remotedl_time_set(INFO_KIND_START, RESET_TIME, &chg_info.dl_info[RESET_TIME].start_time);
		}
		break;
	default:
		err = REQ_NOT_ACCEPT;
		break;
	}
	return err;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�\��m�F
//[]----------------------------------------------------------------------[]
///	@param[in]		*p	: ���u�����e�i���X�v���d��
///	@return			REQ_NOT_ACCEPT = ���s
///					REQ_ACCEPT = ����
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_proc_resv_info(t_ProgDlReq *p)
{
	uchar err = REQ_NOT_ACCEPT;

	// �v�����
	switch (p->ReqKind) {
	case VER_UP_REQ:
	case PROGNO_CHG_REQ:
		// �v����M�ς݁H
		if (remotedl_is_accept(REQ_KIND_VER_UP)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRG_DL_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_DL_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		else {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	case PARAM_CHG_REQ:
		// �v����M�ς݁H
		if (remotedl_is_accept(REQ_KIND_PARAM_CHG)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRM_DL_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRM_DL_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		else {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
// GG120600(S) // MH810100(S) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case PARAM_DIFF_CHG_REQ:
			// �v����M�ς݁H
		if (remotedl_is_accept(REQ_KIND_PARAM_DIF_CHG)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
////			rmon_regist_ex(REQ_KIND_PARAM_DIF_CHG, &p->InstNo1);
//			rmon_regist_ex(RMON_PRM_DIF_DL_CHECK_OK, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
			rmon_regist_ex(RMON_PRM_DIF_DL_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		else {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//// GG120600(S) // MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
////			rmon_regist_ex(REQ_KIND_PARAM_DIF_CHG, &p->InstNo1);
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
//// GG120600(E) // MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
// GG120600(E) // MH810100(E) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case PARAM_UPLOAD_REQ:
		// �v����M�ς݁H
		if (remotedl_is_accept(REQ_KIND_PARAM_UL)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRM_UP_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRM_UP_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		else {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	case RESET_REQ:
		// �v����M�ς݁H
		if (remotedl_is_accept(REQ_KIND_RESET)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_RESET_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_RESET_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		else {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	case PROG_ONLY_CHG_REQ:
		// �v����M�ς݁H
		if (remotedl_is_accept(REQ_KIND_PROG_ONLY_CHG)) {
			err = REQ_ACCEPT;
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_PRG_SW_CHECK_OK, &p->InstNo1);
			rmon_regist_ex(RMON_PRG_SW_CHECK_OK, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		else {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
			rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		}
		break;
	default:
		// �Ď��f�[�^�o�^
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1);
		rmon_regist_ex(RMON_ILLEGAL_REQ_KIND, &p->InstNo1,p->RmonFrom);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	}
	return err;
}
//[]----------------------------------------------------------------------[]
///	@brief			�v���O�����X�V�����̃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		time	: �`�F�b�N�Ώێ���
///	@return			ret		: 0�F�͈͊O(NG) 1�F�͈͓�(OK)
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar remote_date_chk( date_time_rec *pTime )
{
	uchar	ret = 0;
	ushort	ndat;

	// NULL�`�F�b�N
	if (pTime == NULL) {
		return ret;
	}

	// �N�����`�F�b�N
	if( chkdate2( (short)pTime->Year,(short)pTime->Mon,(short)pTime->Day ) == 0 ){
		// ���ԃ`�F�b�N
		if( pTime->Hour <= 23 ){
			if( pTime->Min <= 59 ){
				// ���ݔN������薢���̔N�������󂯕t����
				ndat = dnrmlzm((short)pTime->Year, (short)pTime->Mon, (short)pTime->Day);
				if (CLK_REC.ndat <= ndat) {
					ret = 1;
				}
			}
		}
	}
	return ret;

}
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5796:�O���ȑO�̗\���\�����Ȃ�)
uchar remote_date_chk_mnt( date_time_rec *pTime )
{
	uchar	ret = 0;

	// NULL�`�F�b�N
	if (pTime == NULL) {
		return ret;
	}

	// �N�����`�F�b�N
	if( chkdate2( (short)pTime->Year,(short)pTime->Mon,(short)pTime->Day ) == 0 ) {
		// ���ԃ`�F�b�N
		if( pTime->Hour <= 23 ) {
			if( pTime->Min <= 59 ) {
				ret = 1;
			}
		}
	}
	return ret;

}
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5796:�O���ȑO�̗\���\�����Ȃ�)

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h/���ʏ�񃊃g���C�������菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		time	:FTP�J�n���ԁi���ݎ����Ƃ̔�r�f�[�^�j
///	@param[in]		status	:�X�e�[�^�X���i�X�V�������X�e�[�^�X���w��j
///	@return			ret		: 0�F�`�F�b�NNG 1�F�`�F�b�NOK�iFTP�ʐM�J�n�j
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	dl_start_chk( date_time_rec *time, uchar status )
{
	ulong	GetDate,NowDate;
	uchar	ret = 0;
	
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
//	if (!remote_date_chk(time)) {							// �_�E�����[�h�������s���̏ꍇ�`�F�b�N���Ȃ�
	if (!remote_date_chk_mnt(time)) {
		// �ڑ����g���C�^�C�}�[�҂��̏�Ԃŕ����i�C�x���g�ɂ�菈������Ȃ��悤�ɃK�[�h����
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
		return ret;
	}

	GetDate = Nrm_YMDHM( time );							// �_�E�����[�h�����m�[�}���C�Y
	NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// ���ݎ����m�[�}���C�Y
	if( GetDate <= NowDate ){	 							// ��v�܂��͗\�莞�����o�߂��Ă���
		do {
			// �����e�i���X�����H
			if (OPECTL.Mnt_mod != 0) {
				// �G���[�̊Ď��f�[�^��o�^
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
					rmon_regist(RMON_PRG_DL_START_MNT_NG);
					break;
				case CTRL_PARAM_DL:
					rmon_regist(RMON_PRM_DL_START_MNT_NG);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case CTRL_PARAM_DIF_DL:
					rmon_regist(RMON_PRM_DL_DIF_START_MNT_NG);
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				default:
					break;
				}
				// �J�n���胊�g���C
				if (remotedl_dl_start_retry_check() < 0) {
					// ���g���C�I�[�o�[
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					switch (remotedl_connect_type_get()) {
					case CTRL_PROG_DL:
						rmon_regist(RMON_PRG_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_VER_UP);
						remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
						break;
					case CTRL_PARAM_DL:
						rmon_regist(RMON_PRM_DL_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_CHG);
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
						break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					case CTRL_PARAM_DIF_DL:
						rmon_regist(RMON_PRM_DL_DIF_START_RETRY_OVER);
						remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
						remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
						break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
					default:
						break;
					}
				}
				break;
			}
			// �J�n��������莞�Ԍo�߂��Ă��Ȃ����H
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
//			if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
			if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
//					rmon_regist(RMON_PRG_DL_START_RETRY_OVER);
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
					rmon_regist(RMON_PRG_DL_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
					remotedl_complete_request(REQ_KIND_VER_UP);
					remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
					break;
				case CTRL_PARAM_DL:
//					rmon_regist(RMON_PRM_DL_START_RETRY_OVER);
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
					rmon_regist(RMON_PRM_DL_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
//					rmon_regist(RMON_PRM_DL_DIF_START_RETRY_OVER);
					rmon_regist(RMON_PRM_DL_DIF_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				default:
					break;
				}
				break;
			}

			remotedl_status_set( status );					// FTP�ʐM��Ԃ��X�V
			remotedl_exec_info_set( FLASH_WRITE_BEFORE );	// �_�E�����[�h��Ԃ�������(FLASH�����ݑO�Ƃ���)
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			remotedl_start_retry_clear();					// �J�n���胊�g���C�񐔃N���A
			remotedl_start_retry_clear(connect_type_to_dl_time_kind(remotedl_connect_type_get()));
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

			// �Ď��f�[�^�o�^
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				rmon_regist(RMON_PRG_DL_START_OK);
				break;
			case CTRL_PARAM_DL:
				rmon_regist(RMON_PRM_DL_START_OK);
				break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			case CTRL_PARAM_DIF_DL:
				rmon_regist(RMON_PRM_DL_DIF_START_OK);
				break;				
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			default:
				break;
			}
			queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
			ret = 1;
		} while (0);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�����I������
//[]----------------------------------------------------------------------[]
///	@param[in]		no		:���샂�j�^���
///	@param[in]		result	:���ʏ��
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	moitor_regist( ushort no, uchar result )
{
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���̃����e�i���X�����}�~���
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			ret		:MOD_CHG : mode change<br>
///							:MOD_EXT : F5 key<br>
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
ushort	remotedl_disp( void )
{
	ushort	msg = 0;

	dsp_background_color(COLOR_WHITE);
	dispclr();														// Display All Clear
	grachr(2, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[8]);								/* "�@�T�[�o�[�ƃf�[�^�ʐM���ł��@" */
	grachr(4, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR3[9]);								/* " �����e�i���X����͂ł��܂��� " */
	grachr(5, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[7]);								/* "     ���΂炭���҂�������     " */
	
	
	for( ;; ){
		msg = GetMessage();
		switch( msg ){
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				OPECTL.Ope_mod = 255;							// ��������Ԃ�
				OPECTL.init_sts = 0;							// ��������������ԂƂ���
				OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
				OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
				Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
				OPECTL.Mnt_lev = (char)-1;						// ����ݽ�����߽ܰ�ޖ���
				OPECTL.PasswordLevel = (char)-1;
				return MOD_CUT;
				break;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:										// Mode change key
				if( OPECTL.on_off == 0 ){							// key OFF
// MH810100(S) K.Onodera 2020/03/26 #4101 �Ԕԃ`�P�b�g���X�i���uDL���̃h�A�J��DL�I����ɑ���s�\�ƂȂ�s��C���j
//					OPECTL.Pay_mod = 0;								// �ʏ퐸�Z
//					OPECTL.Mnt_mod = 0;								// ���ڰ���Ӱ�ޏ�����
//					Ope_KeyRepeatEnable(OPECTL.Mnt_mod);
//					OPECTL.Mnt_lev = (char)-1;							// ����ݽ�����߽ܰ�ޖ���
//					OPECTL.PasswordLevel = (char)-1;
					op_wait_mnt_close();							// �����e�i���X�I������
// MH810100(E) K.Onodera 2020/03/26 #4101 �Ԕԃ`�P�b�g���X�i���uDL���̃h�A�J��DL�I����ɑ���s�\�ƂȂ�s��C���j
					return MOD_CHG;
				}
				break;

			case ARC_CR_R_EVT:										// ����IN
				read_sht_opn();										// ���Cذ�ް������J, ���Cذ�ް�޲��LED�_��
				Lagtim( OPETCBNO, 4, 5*50 );
				ope_anm( AVM_CARD_ERR5 );						// �����A����舵�����ł��܂���
				// �����r�o�Ȃ̂ŁA�����ł́A�f���o���̏����͂��Ȃ��B
				break;

			case TIMEOUT4:
			case ARC_CR_EOT_EVT:
				rd_shutter();										// 500msec��ڲ��ɼ��������
				break;

			case REMOTE_DL_END:										// ���u�_�E�����[�h�����I��
				return MOD_EXT;										// ������

			default:
				break;
		}
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�̃t�@�C�����ݒ菈��
//[]----------------------------------------------------------------------[]
///	@param[in/out]	remote	:�T�[�o�[�����t�@�C�����o�b�t�@�|�C���^
///	@param[in/out]	local	:�������z�t�@�C�����o�b�t�@�|�C���^
///	@param[in]		type	:�X�N���v�g�t�@�C�����
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
const char *PATH_DATA[] = {
	"/"								// ���[�g�f�B���N�g��
,	"/TMP/"							// ���z�f�B���N�g��
,	"SCRIPT/"						// �X�N���v�g�t�@�C���p�f�B���N�g��
,	"CUSTOM/"						// �蓮�X�V�p�f�B���N�g��
};

const char *AU_SCRIPT_FILE[] = {
	"FT-4000FX.TXT"					// FT4000FX�W���o�[�W�����A�b�v�p�X�N���v�g
,	(const char *)chg_info.script	// �蓮�X�V�p�iRism����̎w��p�X���g�p�j
,	"update10.txt"					// �����X�V�p�X�N���v�g�t�@�C���i�����t�@�C�����Ȃ̂ŁA�������t�@�C�����Ƃ���j
,	"PARAMETER_UPLOAD.TXT"			// ���ʃp�����[�^�A�b�v���[�h�p�X�N���v�g�t�@�C��
,	"PARAMETER.TXT"					// ���ʃp�����[�^�A�b�v���[�h�t�@�C��
,	"CONNECT.TXT"					// �ڑ��e�X�g
// MH810100(S) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
,	"PARAMETER.DAT"					// LCD�����p�����[�^�A�b�v���[�h
// MH810100(E) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
};

#define SCRIPT_FILE_INDEX_TEST			5

char *RESULT_FILE_PATH[] = {
	"/LOG/RemoteDLResult.txt"											// �����p�t�@�C���p�X�i���L�j
,	"/RESULT/%s%06ld-%02ld_DL_RESULT_%04d%02d%02d%02d%02d.TXT"			// �_�E�����[�h���ʃt�@�C���p�X�{�t�@�C����
,	"/RESULT/%s%06ld-%02ld_SW_RESULT_%04d%02d%02d%02d%02d.TXT"			// �X�V���ʃt�@�C���p�X�{�t�@�C����
,	"/RESULT/%s%06ld-%02ld_CN_RESULT_%04d%02d%02d%02d%02d.TXT"			// �ڑ��m�F�p�t�@�C���p�X�{�t�@�C����
};

void	MakeRemoteFileName( uchar *remote, uchar *local, char type )
{
	uchar	index = 0;
	switch( type ){
		case 0:																// �X�N���v�g�t�@�C�����쐬
			strcpy((char*)local, PATH_DATA[1]);								// ���[�J��(���z)�̃f�B���N�g�����Z�b�g
			strcat((char*)local, AU_SCRIPT_FILE[chg_info.script_type]);		// ���[�J��(���z)�̃t�@�C�������Z�b�g
			
			strcpy((char*)remote, PATH_DATA[0]);							// �t�@�C���p�X�p�ɐ擪��"/"���Z�b�g
			index++;														// �C���f�b�N�X�����炷
			if( chg_info.script_type != MANUAL_SCRIPT ){					// ������FTP�ʐM�ȊO
				strcpy((char*)&remote[index], PATH_DATA[2]);				// �X�N���v�g�̊i�[�f�B���N�g�����Z�b�g
				index += 7;													// �C���f�b�N�X�����炷
			}

			if( chg_info.script_type == REMOTE_MANUAL_SCRIPT ){				// �蓮�X�V��(�ʁ̕W��)
				strcpy((char*)&remote[index], PATH_DATA[3]);				// �蓮�p�̃X�N���v�g�̊i�[�f�B���N�g�����Z�b�g
				index += 7;													// �C���f�b�N�X�����炷
			}
			strcat((char*)&remote[index], AU_SCRIPT_FILE[chg_info.script_type]);	// �_�E�����[�h����X�N���v�g�t�@�C�������Z�b�g
			
			break;
		case 1:																// ���u�_�E�����[�h���ʃt�@�C�����쐬
		case 2:																// �X�V���ʃt�@�C�����쐬
		case 3:																// �ڑ��m�F���쐬
			memcpy( (char*)local, RESULT_FILE_PATH[0], 23);					// ���[�J���i���z�j�p�X���Z�b�g
			sprintf( (char*)remote, RESULT_FILE_PATH[type],					// �A�b�v���[�h����t�@�C�������Z�b�g
							(remotedl_result_get((uchar)(type-1))==0?"":"E"),
// TODO:�f�o�C�XID�͕W���p�̋��ʃp�����[�^���擾���邱��
							0,
							prm_get( COM_PRM,S_PAY,2,2,1 ),					// �@�BNo�Z�b�g
							CLK_REC.year,									// �A�b�v���[�h�N
							CLK_REC.mont,									// �A�b�v���[�h��
							CLK_REC.date,									// �A�b�v���[�h��
							CLK_REC.hour,									// �A�b�v���[�h��
							CLK_REC.minu									// �A�b�v���[�h��
							);
			break;
		case MAKE_FILENAME_SW:			// DL�t�@�C���p�X(CWD)
			local[0] = '\0';

			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
// GG120600(S) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
//// MH810100(S)
////				sprintf((char *)remote, "/BIN/FT4000/%s", chg_info.script);
//				sprintf((char *)remote, "/BIN/GT4100/%s", chg_info.script);
//// MH810100(E)
				sprintf((char *)remote, "/BIN/%03d/%s",NTNET_MODEL_CODE, chg_info.script[PROGNO_KIND_DL]);
// GG120600(E) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
				break;
			case CTRL_PARAM_DL:
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
			case CTRL_PARAM_DIF_DL:
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_PARAM_DOWN,(char*)remote);
				break;
			default:
				break;
			}
			break;
		case MAKE_FILENAME_PARAM_MKD:	// �t�H���_��
			local[0] = '\0';
			if (remotedl_connect_type_get() == CTRL_PARAM_UPLOAD) {
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_PARAM_UP,(char*)remote);
			}
			break;
		case MAKE_FILENAME_PARAM_UP:	// �p�����[�^�A�b�v��
			if (remotedl_connect_type_get() == CTRL_PARAM_UPLOAD) {
				sprintf((char*)local, "/%s", PARAM_FILE_NAME);
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_PARAM_UP,(char*)remote);
				strcat((char*)remote, (char*)local);

				strcpy((char*)local, PATH_DATA[1]);								// ���[�J��(���z)�̃f�B���N�g�����Z�b�g
				strcat((char*)local, AU_SCRIPT_FILE[chg_info.script_type]);		// ���[�J��(���z)�̃t�@�C�������Z�b�g
			}
			break;
		case MAKE_FILENAME_TEST_UP:		// �ڑ��e�X�g�p�t�@�C����
			if (remotedl_connect_type_get() == CTRL_CONNECT_CHK) {
				MakeRemoteFileNameForIP_MODEL_NUM(FILENAME_TEST,(char*)remote);
				strcat((char*)remote, ".tst");

				strcpy((char*)local, PATH_DATA[1]);								// ���[�J��(���z)�̃f�B���N�g�����Z�b�g
				strcat((char*)local, AU_SCRIPT_FILE[SCRIPT_FILE_INDEX_TEST]);	// ���[�J��(���z)�̃t�@�C�������Z�b�g
			}
			break;

// MH810100(S) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
		case MAKE_FILENAME_PARAM_UP_FOR_LCD:
			sprintf( (char*)remote, "%s", PARAM_BIN_FILE_NAME );	// ���M��f�B���N�g��
			strcpy( (char*)local, PATH_DATA[1] );					// ���[�J��(���z)�̃f�B���N�g�����Z�b�g
			strcat( (char*)local, AU_SCRIPT_FILE[LCD_NO_SCRIPT] );	// ���[�J��(���z)�̃t�@�C�������Z�b�g
			break;
// MH810100(E) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
		default:
			break;
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			char*	:�X�N���v�g�t�@�C�����o�b�t�@�|�C���^
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
const char*	remotedl_script_get( void )
{
	return (AU_SCRIPT_FILE[chg_info.script_type]);
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�X�N���v�g��ʐݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		type	: �X�N���v�g���
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_script_typeset( uchar type )
{
	chg_info.script_type = type;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�X�N���v�g��ʎ擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	: 
///	@return			uchar	:�X�N���v�g�擾���
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-04-04<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar	remotedl_script_typeget( void )
{
	return	chg_info.script_type;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h�t���b�v���쒆�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			ret		: 0�F���쒆�̃t���b�v�Ȃ� 1�F����
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	flap_move_chk( void )
{
	short i;
	uchar ret = 0;
	
	for( i = 0; i < LOCK_MAX; i++ ){
		WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( FLPCTL.Flp_mv_tm[i] != 0 && FLPCTL.Flp_mv_tm[i] != -1 ){
			ret = 1;
			break;
		}
	}
	return ret;
}															

//[]----------------------------------------------------------------------[]
///	@brief			���݃��g���C���쒆�̋@�\��ʔ���
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			status	: ���g���C�X�e�[�^�X 0�F���g���C���ł͂Ȃ� ��0 ���g���C��
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar now_retry_active_chk(uchar kind)
{
	int i;
	uchar ret = 0;

	if (kind == RETRY_KIND_MAX) {
		for (i = 0; i < RETRY_KIND_MAX; i++) {
			if (chg_info.retry[i].status&0x7f) {
				ret = 1;
				break;
			}
		}
	}
	else {
		if (chg_info.retry[kind].status&0x7f) {
			ret = 1;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�_�E�����[�h�O�̃G���[�������̏���
//[]----------------------------------------------------------------------[]
///	@param[in]		Err		:�G���[���
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	Before_DL_Err_Function( uchar Err )
{
	uchar status = remotedl_status_get();
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
	uchar kind;
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

	remotedl_result_set( Err );

	switch( status ){
		case	R_DL_EXEC:
			moitor_regist(OPLOG_REMOTE_DL_END, Err);
			if( remotedl_connect_type_get() == CTRL_CONNECT_CHK){
//				g_bk_ProgDlReq.ReqResult = REQ_CONN_NG;
//				NTNET_Snd_Data118_DL(&g_bk_ProgDlReq);
//				memset(&g_bk_ProgDlReq,0, sizeof(g_bk_ProgDlReq));
				rmon_regist(RMON_FTP_TEST_RES_NG);					// FTP�ڑ��e�X�g���s
				remotedl_status_set(R_DL_IDLE);
				remotedl_chg_info_restore();						// ��񃊃X�g�A
				break;
			}
			// �G���[�̊Ď��f�[�^��o�^
			rmon_regist(RMON_FTP_LOGIN_ID_PW_NG);
			// ���g���C
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			if (retry_info_set(RETRY_KIND_CONNECT)) {
			switch (remotedl_connect_type_get()) {
			case CTRL_PROG_DL:
				kind = RETRY_KIND_CONNECT_PRG;
				break;
			case CTRL_PARAM_DL:
				kind = RETRY_KIND_CONNECT_PARAM_DL;
				break;
			case CTRL_PARAM_DIF_DL:
				kind = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
				break;
			case CTRL_PARAM_UPLOAD:
				kind = RETRY_KIND_CONNECT_PARAM_UP;
				break;
			default:
				return;
				break;
			}
			if (retry_info_set(kind)) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				// ���g���C���N���A
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				retry_info_clr(RETRY_KIND_MAX);
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
					retry_info_clr(RETRY_KIND_CONNECT_PRG);
					retry_info_clr(RETRY_KIND_DL_PRG);
					break;
				case CTRL_PARAM_DL:
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
					retry_info_clr(RETRY_KIND_DL_PARAM);
					break;
				case CTRL_PARAM_DIF_DL:
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
					retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
					break;
				case CTRL_PARAM_UPLOAD:
					retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
					retry_info_clr(RETRY_KIND_UL);
					break;
				}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
				// �󂯕t�����v�����N���A����
				switch (remotedl_connect_type_get()) {
				case CTRL_PROG_DL:
					rmon_regist(RMON_PRG_DL_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_VER_UP);
					remotedl_comp_set(INFO_KIND_START, PROG_DL_TIME, EXEC_STS_ERR);
					break;
				case CTRL_PARAM_DL:
					rmon_regist(RMON_PRM_DL_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_TIME, EXEC_STS_ERR);
					break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case CTRL_PARAM_DIF_DL:
					rmon_regist(RMON_PRM_DL_DIF_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
					remotedl_comp_set(INFO_KIND_START, PARAM_DL_DIF_TIME, EXEC_STS_ERR);
					break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
				case CTRL_PARAM_UPLOAD:
					rmon_regist(RMON_PRM_UP_END_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_UL);
					remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
					break;
				default:
					break;
				}
			}
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//			remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
			break;


		default:
			break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�ċN�����̌��ʏ�񑗐M�҂�����
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		SUBCPU�͋N�������莞�ԊԊu���󂯂Ă���łȂ����FTP<br>
///					�ʐM�����肵�Ȃ��ׁA�N������90�b��ɐڑ����ɂ���
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remote_result_snd_timer( void )
{
}

//[]----------------------------------------------------------------------[]
///	@brief			Rism�ڑ��ؒf�҂���ѱ��
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	connect_timeout( void )
{
	queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
}

//[]----------------------------------------------------------------------[]
///	@brief			Rism�ڑ��ؒf�v������
//[]----------------------------------------------------------------------[]
///	@param[in]		logno	: ���샂�j�^��ʁi�ؒfor�ڑ��j
///	@param[in]		type	: �ڑ�/�ؒf���(�ڑ�/�ؒf���s���v��)
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_connect_req( ushort logno, uchar type )
{
	remotedl_connect_type_set( type );
	connect_timeout();
}

/*[]----------------------------------------------------------------------[]*/
///	@brief			Ұٷ������_����Ұق��擾����i�g���j
//[]----------------------------------------------------------------------[]
///	@param[in]		id		: �擾������ް
///	@param[in]		pReq	: �����Ώ�ү����ID�iү���ތS�̑�\�ԍ����w��j
///	@return			MsgBuf	: �擾ү�����߲��,�擾ү���ނ��Ȃ����NULL
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
MsgBuf	*Target_MsgGet_Range( uchar id, t_TARGET_MSGGET_PRM *pReq )
{
	MsgBuf	*msg_add;										// Ұٷ���������Ώ�Ұٱ��ڽ��Ĵر
	ushort	MailCommand;									// Ұٷ���������Ώ�Ұق�ү����ID
	ushort	ReqCount;										// �v�����ꂽ�����Ώ�ү����ID��
	ulong	ist;											// ���݂̊�����t���
	uchar	i,j;
	MsgBuf	*msg_add_Prev;									// Ұٷ����(1��O)�����Ώ�Ұٱ��ڽ��Ĵر

	if( tcb[id].msg_top == NULL ) {							// ҰقȂ�
		return( NULL );
	}

	ReqCount = pReq->Count;									// �v�����ꂽ�����Ώ�ү����ID��get

	ist = _di2();											// �����݋֎~
	msg_add = tcb[id].msg_top;								// �����Ώ�Ұٱ��ڽget�i�ŏ��͐擪�j

	// ��������
	for( i=0; i<MSGBUF_CNT; ++i ){							// Ұٷ���� �SҰ������iMSGBUF_CNT�͒P��Limitter�j

		MailCommand = msg_add->msg.command;					// �����Ώ�Ұق�ү����IDget

		for( j=0; j<ReqCount; ++j ){						// �T���ė~����Ұ�ID����
			if(( pReq->Command[j] & 0x000f) == 0 ){			// �v���R�[�h�̉��P�����O�i��\�j�̏ꍇ
				if( pReq->Command[j] == (MailCommand&0xff00) ){	// ��2�����}�X�N�����l���A��\(�O���[�v)��v
					goto Target_MsgGet_10;						// ������Ұق𷭰���甲��
				}
			}else{											// �v���R�[�h���}�Ԃ̏ꍇ
				if( pReq->Command[j] == MailCommand ){		// �}�Ԃ܂ň�v���Ă��邩�𔻒�
					goto Target_MsgGet_10;					// ������Ұق𷭰���甲��
				}
			}
		}

		// ����Ұقցi���������j
		msg_add_Prev = msg_add;								// �OҰٱ��ڽ���ށi�������ɕK�v�j
		msg_add = (MsgBuf*)(msg_add->msg_next);				// �������Ώ�Ұٱ��ڽget
		if( NULL == msg_add ){								// ��ҰقȂ�
			break;											// �����I��
		}

		// 16���1�� WDT�ر
		if( 0x0f == (i & 0x0f) ){
			WACDOG;
		}
	}

	// �����ł��Ȃ������ꍇ�i�c��ҰقȂ��j
	_ei2( ist );
	return( NULL );

	// ���������ꍇ
Target_MsgGet_10:
	// Target message�𷭰���甲��

	if( 0 == i ){											// �擪Ұق̏ꍇ
		tcb[id].msg_top = (MsgBuf *)tcb[id].msg_top->msg_next;
		if( tcb[id].msg_top == NULL ) {						// Ұق�1�������Ȃ������ꍇ
			tcb[id].msg_end = NULL;
			tcb[id].event = MSG_EMPTY;
		}
	}
	else if( tcb[id].msg_end == msg_add ){					// ����Ұق̏ꍇ�i2���ȏ�Ұق�����j
		msg_add_Prev->msg_next = NULL;						// �ŏIҰ�ϰ�set
		tcb[id].msg_end = msg_add_Prev;
	}
	else{													// �r��Ұق̏ꍇ�i2���ȏ�Ұق�����j
		msg_add_Prev->msg_next = msg_add->msg_next;			// �ŏIҰ�ϰ�set
	}

	_ei2( ist );
	return( msg_add );
}

//[]----------------------------------------------------------------------[]
///	@brief			�ؑ֏��̃o�b�N�A�b�v����
//[]----------------------------------------------------------------------[]
///	@param[in]		dat		: �ؑ֏��|�C���^
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_sw_update_bk( uchar *dat )
{
	memcpy(chg_info.sw_status, dat, 3);
}

//[]----------------------------------------------------------------------[]
///	@brief			�ؑ֏��̎擾�i�╜�d�y�ь��ʏ�񃊃g���C���Ɏ擾�j
//[]----------------------------------------------------------------------[]
///	@param[in]		dat		: �ؑ֏��|�C���^
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_sw_update_get( uchar *dat )
{
	memcpy(dat, chg_info.sw_status, 3);
}

//[]----------------------------------------------------------------------[]
///	@brief			�_�E�����[�h���̃o�b�N�A�b�v
//[]----------------------------------------------------------------------[]
///	@param[in]		void	: 
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_chg_info_bkup( void )
{
	memcpy(&bk_chg_info, &chg_info, sizeof( t_prog_chg_info ));
}

//[]----------------------------------------------------------------------[]
///	@brief			�_�E�����[�h���̃��X�g�A
//[]----------------------------------------------------------------------[]
///	@param[in]		void	: 
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-09<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	remotedl_chg_info_restore( void )
{
	memcpy(&chg_info, &bk_chg_info, sizeof( t_prog_chg_info ));	// �_�E�����[�h���̃��X�g�A
	memset(&bk_chg_info, 0, sizeof( t_prog_chg_info ));		// ���X�g�A�������ɂ̓o�b�N�A�b�v�G���A������
}

//[]----------------------------------------------------------------------[]
///	@brief			���ʃp�����[�^�A�b�v���[�h��ʐݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		type	:��ʎ�� 0�F�V���[�g�J�b�g 1�F�}�j���A������
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
void	Param_Upload_type_set( uchar type )
{
	chg_info.param_up = type;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ʃp�����[�^�A�b�v���[�h��ʎ擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:��ʎ�� 0�F�}�j���A������ 1�F�V���[�g�J�b�g
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-12-10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
uchar	Param_Upload_type_get( void )
{
	return	(chg_info.param_up);
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���s�󋵍X�V
//[]----------------------------------------------------------------------[]
///	@param[in]		sts		:�_�E�����[�h���s��
///	@return			void	:
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2011-09-26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void	remotedl_exec_info_set( uchar exec )
{
	chg_info.exec_info = exec;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�_�E�����[�h���s�󋵎Q��
//[]----------------------------------------------------------------------[]
///	@param[in]		void	:	
///	@return			status	:�_�E�����[�h���s��
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2011-09-26<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
uchar	remotedl_exec_info_get( void )
{
	return	(chg_info.exec_info);
}

//[]----------------------------------------------------------------------[]
///	@brief			�ݒ�A�b�v���[�h�J�n����
//[]----------------------------------------------------------------------[]
///	@param[in]		*time	: �ݒ�A�b�v���[�h�J�n����
///	@param[in]		status	: �X�V����X�e�[�^�X
///	@return			ret		: 0=�J�n���s, 1=�J�n����
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar	up_wait_chk( date_time_rec *time, uchar status )
{
	ulong	GetDate,NowDate;
	uchar	ret = 0;

// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
//	if (!remote_date_chk(time)) {							// �A�b�v���[�h�������s���̏ꍇ�`�F�b�N���Ȃ�
	if (!remote_date_chk_mnt(time)) {
		// �ڑ����g���C�^�C�}�[�҂��̏�Ԃŕ����i�C�x���g�ɂ�菈������Ȃ��悤�ɃK�[�h����
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
		return ret;
	}

	GetDate = Nrm_YMDHM( time );							// �A�b�v���[�h�����m�[�}���C�Y
	NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// ���ݎ����m�[�}���C�Y

	if( GetDate <= NowDate ){	 							// ��v�܂��͗\�莞�����o�߂��Ă���
		do {
			// �����e�i���X�����H
// GM760201(S) LCD�ɐݒ�A�b�v���[�h���́A�Z���^�[�A�b�v���[�h���Ȃ�
//			if (OPECTL.Mnt_mod != 0) {
			if ((OPECTL.Mnt_mod != 0) || (OPECTL.lcd_prm_update != 0)){
// GM760201(E) LCD�ɐݒ�A�b�v���[�h���́A�Z���^�[�A�b�v���[�h���Ȃ�
				rmon_regist(RMON_PRM_UP_START_MNT_NG);
				if (remotedl_dl_start_retry_check() < 0) {
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//					remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//					remotedl_start_retry_clear();
					remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
					rmon_regist(RMON_PRM_UP_START_RETRY_OVER);
					remotedl_complete_request(REQ_KIND_PARAM_UL);
					remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
				}
				break;
			}
			// �J�n��������莞�Ԍo�߂��Ă��Ȃ����H
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
//			if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
			if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
//				rmon_regist(RMON_PRM_UP_START_RETRY_OVER);
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
				rmon_regist(RMON_PRM_UP_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
				remotedl_complete_request(REQ_KIND_PARAM_UL);
				remotedl_comp_set(INFO_KIND_START, PARAM_UP_TIME, EXEC_STS_ERR);
				break;
			}

			remotedl_status_set( status );					// FTP�ʐM��Ԃ��X�V
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			remotedl_start_retry_clear();					// �J�n���胊�g���C�񐔃N���A
			remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

			rmon_regist(RMON_PRM_UP_START_OK);

			// �A�b�v���[�h�J�n
			queset( OPETCBNO, REMOTE_CONNECT_EVT, 0, NULL );
			ret = 1;
		} while (0);
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���Z�b�g�J�n����
//[]----------------------------------------------------------------------[]
///	@param[in]		*time	: ���Z�b�g�J�n����
///	@param[in]		status	: �X�V����X�e�[�^�X
///	@return			ret		: 0=�J�n���s, 1=�J�n����
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar	reset_start_chk( date_time_rec *time, uchar status )
{
	ulong	GetDate,NowDate;
	uchar	ret = 0;
	ulong	code;

	GetDate = Nrm_YMDHM( time );							// ���Z�b�g�����m�[�}���C�Y
	NowDate = Nrm_YMDHM( (date_time_rec*)&CLK_REC );		// ���݂̎����m�[�}���C�Y
	if( GetDate <= NowDate ){	 							// ��v�܂��͗\�莞�����o�߂��Ă���
		do {
			// �����e�i���X�����ҋ@���x�ƈȊO�̏ꍇ
			if (!(OPECTL.Mnt_mod == 0 &&
				(OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100))) {
				code = RMON_RESET_START_STATUS_NG;
				break;
			}
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//			// Edy�����Z���^�[�ʐM���A����T���v���{��
//			if ((auto_cnt_prn == 2) ||
//				(auto_syu_prn == 1) || (auto_syu_prn == 2)) {
			// ����T���v���{��
			if ((auto_syu_prn == 1) || (auto_syu_prn == 2)) {
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
				code = RMON_RESET_START_COMM_NG;
				break;
			}
			// �t���b�v���쒆
			if (flap_move_chk()) {
				code = RMON_RESET_START_FLAP_NG;
				break;
			}
			// �h�A���J���Ă��邩�A�\���m�C�h���b�N���������
			if (DOOR_OPEN_OR_SOLENOID_UNROCK()) {
				code = RMON_RESET_START_DOOR_OPEN_NG;
				break;
			}
			// �J�n��������莞�Ԍo�߂��Ă��Ȃ����H
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
//			if ((NowDate-GetDate) >= NG_ELAPSED_TIME) {
			if (remotedl_nrm_time_sub(GetDate, NowDate) >= NG_ELAPSED_TIME) {
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//				remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//				remotedl_start_retry_clear();
				remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
//				rmon_regist(RMON_RESET_START_RETRY_OVER);
// GG120600(S) // Phase9 (��莞�Ԍo��)��ǉ�
				rmon_regist(RMON_RESET_START_OVER_ELAPSED_TIME);
// GG120600(E) // Phase9 (��莞�Ԍo��)��ǉ�
				remotedl_complete_request(REQ_KIND_RESET);
				remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_ERR);
				return 0;
			}

			remotedl_status_set( status );					// FTP�ʐM��Ԃ��X�V
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			remotedl_start_retry_clear();					// �J�n���胊�g���C�񐔃N���A
			remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

			rmon_regist(RMON_RESET_START_OK);

			// ���Z�b�g�J�n
			System_reset();
			ret = 1;
		} while (0);

		// �G���[�̊Ď��f�[�^��o�^
		rmon_regist(code);

		// ���Z�b�g�J�nNG
		if (remotedl_reset_start_retry_check() < 0) {
// GG120600(S) // Phase9 ��ʖ����`�F�b�N
//			remotedl_status_set(R_DL_IDLE);
// GG120600(E) // Phase9 ��ʖ����`�F�b�N
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//			remotedl_start_retry_clear();
			remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
			rmon_regist(RMON_RESET_START_RETRY_OVER);
			remotedl_complete_request(REQ_KIND_RESET);
			remotedl_comp_set(INFO_KIND_START, RESET_TIME, EXEC_STS_ERR);
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���O�쐬�iIP+�@��+�@�B��)
//[]----------------------------------------------------------------------[]
///	@param[in]		status	:�X�e�[�^�X���i�X�V�������X�e�[�^�X���w��j
///	@return			ret		: 0�F�`�F�b�NNG 1�F�`�F�b�NOK�iFTP�ʐM�J�n�j
///	@attention		None
//[]----------------------------------------------------------------------[]
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
static void MakeRemoteFileNameForIP_MODEL_NUM(ushort updown,char *remote)
{
// GG120600(S) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
//	if( updown == FILENAME_PARAM_UP ){
//		if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 ){				// LAN�ȊO
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/UPLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/UPLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_CEN][65],
//				CPrmSS[S_CEN][66],
//				NTNET_MODEL_CODE,						// �@��
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
//				);
//		}else{
//			// LAN�̏ꍇ�i�b��E�E�E192.168���ł͂Ԃ���\������j
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/UPLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/UPLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_MDL][2],
//				CPrmSS[S_MDL][3],
//				NTNET_MODEL_CODE,						// �@��
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
//				);
//		}
//	}else if(updown == FILENAME_PARAM_DOWN){
//		if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 ){				// LAN�ȊO
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/DOWNLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/DOWNLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_CEN][65],
//				CPrmSS[S_CEN][66],
//				NTNET_MODEL_CODE,						// �@��
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
//				);
//		}else{
//			// LAN�̏ꍇ�i�b��E�E�E192.168���ł͂Ԃ���\������j
//// MH810100(S)
////			sprintf((char *)remote, "/PARAM/FT4000/DOWNLOAD/%06ld%06ld%03d%02ld",
//			sprintf((char *)remote, "/PARAM/GT4100/DOWNLOAD/%06ld%06ld%03d%02ld",
//// MH810100(E)
//				CPrmSS[S_MDL][2],
//				CPrmSS[S_MDL][3],
//				NTNET_MODEL_CODE,						// �@��
//				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
//				);
//		}
//	}else if(updown == FILENAME_TEST){
//// MH810100(S)
////		sprintf((char *)remote, "/TEST/FT4000/%06ld%06ld%03d%02ld",
//		sprintf((char *)remote, "/TEST/GT4100/%06ld%06ld%03d%02ld",
//// MH810100(E)
//			CPrmSS[S_CEN][65],
//			CPrmSS[S_CEN][66],
//			NTNET_MODEL_CODE,						// �@��
//			prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
//			);
//	}

	char	pathname[256];
	uchar	ipaddr_tmp[4];
	int 	i;

	memset(pathname,0,sizeof(pathname));
	if( prm_get( COM_PRM,S_CEN,51,1,3 ) != 1 ){				// LAN�ȊO
		sprintf((char *)pathname, "%06ld%06ld%03d%02ld",
				CPrmSS[S_CEN][65],
				CPrmSS[S_CEN][66],
				NTNET_MODEL_CODE,						// �@��
				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
				);
	}else{
		// FTP�p�T�[�o�[�t�H���_IP�����邩�ǂ����`�F�b�N
		for (i = 0; i < 4; i++) {
			ipaddr_tmp[i] = (uchar)prm_get( COM_PRM,S_RDL,7 + i,3,1 );
		}
		// �Ȃ�������LAN���g�p����
		if (*((ulong *)ipaddr_tmp) == 0) {
			sprintf((char *)pathname, "%06ld%06ld%03d%02ld",
				CPrmSS[S_MDL][2],
				CPrmSS[S_MDL][3],
				NTNET_MODEL_CODE,						// �@��
				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
				);
		}else{
			sprintf((char *)pathname, "%03d%03d%03d%03d%03d%02ld",
				ipaddr_tmp[0],ipaddr_tmp[1],ipaddr_tmp[2],ipaddr_tmp[3],
				NTNET_MODEL_CODE,						// �@��
				prm_get(COM_PRM, S_PAY, 2,  2, 1)		// �@�B��
				);
		}

	}
	if( updown == FILENAME_PARAM_UP ){
		sprintf((char *)remote, "/PARAM/%03d/UPLOAD/%s",NTNET_MODEL_CODE,pathname);
	}else if(updown == FILENAME_PARAM_DOWN){
		sprintf((char *)remote, "/PARAM/%03d/DOWNLOAD/%s",NTNET_MODEL_CODE,pathname);
	}else if(updown == FILENAME_TEST){
		sprintf((char *)remote, "/TEST/%03d/%s",NTNET_MODEL_CODE,pathname);
	}

// GG120600(E) // Phase9 �T�[�o�t�H���_�\���ύX�Ή�
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP�ڑ��e�X�g�t�@�C�����e�쐬
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*pBuff	: �t�@�C���o�b�t�@
///	@param[in]		nSize	: �t�@�C���o�b�t�@�T�C�Y
///	@return			ret		: �t�@�C���T�C�Y
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ushort AppServ_MakeRemoteDl_TestConnect(char* pBuff,int nSize)
{
	ushort nFileSize;
	if( nSize < TEST_CONNECT_FILE_SIZE){
		nFileSize = nSize;
	}else{
		nFileSize = TEST_CONNECT_FILE_SIZE;
	}
	memset( pBuff, 0x31,nFileSize);
	
	return nFileSize;
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP�T�[�o�A�h���X�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: FTP�T�[�o�A�h���X(���l)
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ulong remotedl_ftp_ipaddr_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_DIF_DL:
		type = PARAM_DL_DIF_TIME;
		break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return 0;
	}
	return chg_info.dl_info[type].ftpaddr.uladdr;
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP�|�[�g�ԍ��擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: FTP�|�[�g�ԍ�
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ushort remotedl_ftp_port_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_DIF_DL:
		type = PARAM_DL_DIF_TIME;
		break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return 0;
	}
	return chg_info.dl_info[type].ftpport;
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP���[�U�擾
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*user	: FTP���[�U������
///	@param[in]		size	: FTP���[�U������T�C�Y
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_ftp_user_get(char *user, int size)
{
	uchar type;

	if (user) {
		switch (remotedl_connect_type_get()) {
		case CTRL_PROG_DL:
			type = PROG_DL_TIME;
			break;
		case CTRL_PARAM_DL:
			type = PARAM_DL_TIME;
			break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_DIF_DL:
			type = PARAM_DL_DIF_TIME;
			break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_UPLOAD:
			type = PARAM_UP_TIME;
			break;
		case CTRL_CONNECT_CHK:
			type = TEST_TIME;
		break;
		default:
			user[0] = '\0';
			return;
		}
		strncpy(user, (char *)chg_info.dl_info[type].ftpuser, size);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP�p�X���[�h�擾
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*passwd	: FTP�p�X���[�h������
///	@param[in]		size	: FTP�p�X���[�h������T�C�Y
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_ftp_passwd_get(char *passwd, int size)
{
	uchar type;

	if (passwd) {
		switch (remotedl_connect_type_get()) {
		case CTRL_PROG_DL:
			type = PROG_DL_TIME;
			break;
		case CTRL_PARAM_DL:
			type = PARAM_DL_TIME;
			break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_DIF_DL:
			type = PARAM_DL_DIF_TIME;
			break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case CTRL_PARAM_UPLOAD:
			type = PARAM_UP_TIME;
			break;
		case CTRL_CONNECT_CHK:
			type = TEST_TIME;
		break;
		default:
			passwd[0] = '\0';
			return;
		}
		strncpy(passwd, (char *)chg_info.dl_info[type].ftppasswd, size);
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			DL�EUL�J�n���胊�g���C�񐔃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=���g���C��, -1=���g���C�I�[�o�[
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_dl_start_retry_check(void)
{
	int ret = 0;
	ushort retry_cnt;

	// �_�E�����[�h�E�A�b�v���[�h�J�n���胊�g���C��
	retry_cnt = (ushort)prm_get(COM_PRM, S_RDL, 3, 3, 1);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	if (++chg_info.retry_dl_cnt > retry_cnt) {
	if (++chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(remotedl_connect_type_get())] > retry_cnt) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�X�V�J�n���胊�g���C�񐔃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=���g���C��, -1=���g���C�I�[�o�[
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_sw_start_retry_check(void)
{
	int ret = 0;
	ushort retry_cnt;

	// �v���O�����؊��J�n���胊�g���C��
	retry_cnt = (ushort)prm_get(COM_PRM, S_RDL, 4, 3, 1);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	if (++chg_info.retry_dl_cnt > retry_cnt) {
	if (++chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(remotedl_connect_type_get())] > retry_cnt) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���Z�b�g�J�n���胊�g���C�񐔃`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=���g���C��, -1=���g���C�I�[�o�[
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_reset_start_retry_check(void)
{
	int ret = 0;
	ushort retry_cnt;

	// ���Z�b�g�J�n���胊�g���C��
	retry_cnt = (ushort)prm_get(COM_PRM, S_RDL, 6, 3, 1);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	if (++chg_info.retry_dl_cnt > retry_cnt) {
	if (++chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(remotedl_connect_type_get())] > retry_cnt) {
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�J�n���胊�g���C�񐔃N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//void remotedl_start_retry_clear(void)
void remotedl_start_retry_clear(uchar dl_kind)
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
{
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	chg_info.retry_dl_cnt = 0;
	if (dl_kind < TIME_INFO_MAX) {
		chg_info.retry_dl_cnt[dl_kind] = 0;
	}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
}

//[]----------------------------------------------------------------------[]
///	@brief			�t�F�[���Z�[�t�^�C�}�N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_failsafe_timer_clear(void)
{
	chg_info.failsafe_timer = 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�t�F�[���Z�[�t�^�C�}�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 0=�^�C�}���쒆, -1=�^�C�}����
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_failsafe_timer_check(void)
{
	int ret = 0;
	ushort failsafe;

	failsafe = (ushort)prm_get(COM_PRM, S_RDL, 2, 3, 1);
	if (!failsafe) {
		failsafe = 30;
	}
	if (++chg_info.failsafe_timer > failsafe) {
		ret = -1;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X������Ԏ擾(�v�������̒l�ɕϊ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ���u�����e�i���X�������
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_convert_dl_status(void)
{
	int ret = REQ_NONE;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:					// *** �v���O�����_�E�����[�h ***
		{
			switch (remotedl_status_get()) {
			case R_DL_IDLE:
			case R_DL_REQ_RCV:
				ret = REQ_PROG_DL_WAIT;
				break;
			case R_DL_START:
			case R_DL_EXEC:
				ret = REQ_PROG_DL_EXEC;
				break;
			case R_SW_WAIT:
				ret = REQ_PROG_SW_WAIT;
				break;
			default:
				break;
			}
		}
		break;
	case CTRL_PARAM_DL:					// *** �ݒ�_�E�����[�h ***
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_DIF_DL:
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		{
			switch (remotedl_status_get()) {
			case R_DL_IDLE:
			case R_DL_REQ_RCV:
				ret = REQ_PARAM_DL_WAIT;
				break;
			case R_DL_START:
			case R_DL_EXEC:
				ret = REQ_PARAM_DL_EXEC;
				break;
			case R_SW_WAIT:
				ret = REQ_PARAM_SW_WAIT;
				break;
			default:
				break;
			}
		}
		break;
	case CTRL_RESET:					// *** ���Z�b�g ***
		{
			ret = REQ_RESET_WAIT;
		}
		break;
	case CTRL_PARAM_UPLOAD:				// **** �ݒ�A�b�v���[�h ***
		{
			switch (remotedl_status_get()) {
			case R_DL_IDLE:
			case R_UP_WAIT:
				ret = REQ_PARAM_UL_WAIT;	// �ݒ�A�b�v���[�h�҂�
				break;
			case R_UP_START:
			case R_DL_EXEC:
				ret = REQ_PARAM_UL_EXEC;	// �ݒ�A�b�v���[�h��
				break;
			default:
				break;
			}
		}
		break;
	case CTRL_CONNECT_CHK:				// *** �ڑ��e�X�g ***
		{
			ret = REQ_TEST_NOW;			// �ڑ��e�X�g��
		}
		break;
	case CTRL_PROG_SW:					// *** �v���O�����؊� ***
		{
			ret = REQ_PROG_SW_WAIT;		// �v���O�����X�V�J�n�҂�
		}
		break;
	case CTRL_NONE:
	default:
		ret = REQ_RCV_NONE;
		break;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�v����t
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: �v�����
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_accept_request(int request)
{
	chg_info.req_accept |= request;

// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
	// ���Ɏ��s�����v�����Z�b�g
	remotedl_arrange_next_request();
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�v���N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: �v�����
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_clear_request(int request)
{
	chg_info.req_accept &= ~request;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�v����t�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: �v�����
///	@return			ret		: FALSE=��t����, TRUE=��t����
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
uchar remotedl_is_accept(int request)
{
	uchar ret = FALSE;

	if ((chg_info.req_accept & request) == request) {
		ret = TRUE;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�v������
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: �v�����
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_complete_request(int request)
{
	remotedl_connect_type_set(CTRL_NONE);
	remotedl_clear_request(request);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	remotedl_start_retry_clear();
//	retry_info_clr(RETRY_KIND_MAX);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
	remotedl_monitor_info_set(0);

	// �o�[�W�����A�b�v�v��
	if (request & REQ_KIND_VER_UP) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		chg_info.result[RES_DL] = EXCLUDED;
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_DL_PROG] = EXCLUDED;
		chg_info.result[RES_SW_PROG] = EXCLUDED;
// GG120600(E) // Phase9 ���ʂ���ʂ���
		remotedl_recv_info_clear(PROG_DL_TIME);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
		retry_info_clr(RETRY_KIND_CONNECT_PRG);
		retry_info_clr(RETRY_KIND_DL_PRG);
		remotedl_start_retry_clear(PROG_DL_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		chg_info.update = 0;
	}

	// �ݒ�ύX�v��
	if (request & REQ_KIND_PARAM_CHG) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		chg_info.result[RES_DL] = EXCLUDED;
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_DL_PARAM] = EXCLUDED;
		chg_info.result[RES_SW_PARAM] = EXCLUDED;
// GG120600(E) // Phase9 ���ʂ���ʂ���
		remotedl_recv_info_clear(PARAM_DL_TIME);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
		retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
		retry_info_clr(RETRY_KIND_DL_PARAM);
		remotedl_start_retry_clear(PARAM_DL_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		chg_info.update = 0;
	}

// MH810100(S)
	if (request & REQ_KIND_PARAM_DIF_CHG) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		chg_info.result[RES_DL] = EXCLUDED;
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_DL_PARAM_DIF] = EXCLUDED;
		chg_info.result[RES_SW_PARAM_DIF] = EXCLUDED;
// GG120600(E) // Phase9 ���ʂ���ʂ���
		remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
		retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
		retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
		remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		chg_info.update = 0;
	}
// MH810100(E)

	// �ݒ�v��
	if (request & REQ_KIND_PARAM_UL) {
		chg_info.result[RES_UP] = EXCLUDED;
		remotedl_recv_info_clear(PARAM_UP_TIME);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
		retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
		retry_info_clr(RETRY_KIND_UL);
		remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
	}

	// ���Z�b�g�v��
	if (request & REQ_KIND_RESET) {
		remotedl_recv_info_clear(RESET_TIME);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
		remotedl_start_retry_clear(RESET_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
	}

	// �v���O�����؊��v��
	if (request & REQ_KIND_PROG_ONLY_CHG) {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//		chg_info.result[RES_SW] = EXCLUDED;
		chg_info.result[RES_SW_PROG] = EXCLUDED;
// GG120600(E) // Phase9 ���ʂ���ʂ���
		remotedl_recv_info_clear(PROG_ONLY_TIME);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
		remotedl_start_retry_clear(PROG_ONLY_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
		chg_info.update = 0;
	}

	// �ڑ��e�X�g
	if (request & REQ_KIND_TEST) {
		chg_info.result[RES_COMM] = EXCLUDED;
		remotedl_recv_info_clear(TEST_TIME);
	}
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
	// ���Ɏ��s�����v�����Z�b�g
	remotedl_arrange_next_request();
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�v����M���N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		type	: �v�����
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_recv_info_clear(uchar type)
{
	if (type < TIME_MAX) {
		// �Ώێ�ʂ̕ێ������N���A
		memset(&chg_info.dl_info[type], 0, sizeof(t_dl_info));
		switch (type) {
		case PROG_DL_TIME:
			memset(&chg_info.sw_info[SW_PROG], 0, sizeof(t_sw_info));
			chg_info.script[PROGNO_KIND_DL][0] = '\0';
			remotedl_write_info_clear();
			break;
		case PARAM_DL_TIME:
			memset(&chg_info.sw_info[SW_PARAM], 0, sizeof(t_sw_info));
			break;
// MH810100(S) Y.Yamauchi 20191224 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case PARAM_DL_DIF_TIME:
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//			memset(&chg_info.sw_info[SW_PARAM], 0, sizeof(t_sw_info));
			memset(&chg_info.sw_info[SW_PARAM_DIFF], 0, sizeof(t_sw_info));
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
			break;
// MH810100(E) Y.Yamauchi 20191224 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
		case PROG_ONLY_TIME:
			memset(&chg_info.sw_info[SW_PROG_EX], 0, sizeof(t_sw_info));
			chg_info.script[PROGNO_KIND_SW][0] = '\0';
			break;
		default:
			break;
		}
	}
	else {
		// ���ׂăN���A
		memset(chg_info.dl_info, 0, sizeof(t_prog_chg_info)-offsetof(t_prog_chg_info, dl_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���ߔԍ��擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: ���ߔԍ�
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
u_inst_no* remotedl_instNo_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 �����_�E�����[�h���ɖ��ߔԍ�����ɂȂ�
//		type = PARAM_DL_TIME;
		type = PARAM_DL_DIF_TIME;
// GG120600(E) // Phase9 �����_�E�����[�h���ɖ��ߔԍ�����ɂȂ�
		break;
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_RESET:
		type = RESET_TIME;
		break;
	case CTRL_PROG_SW:
		type = PROG_ONLY_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return NULL;
	}
	return &chg_info.dl_info[type].u;
}

// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
uchar remotedl_instFrom_get(void)
{
	uchar type;

	switch (remotedl_connect_type_get()) {
	case CTRL_PROG_DL:
		type = PROG_DL_TIME;
		break;
	case CTRL_PARAM_DL:
		type = PARAM_DL_TIME;
		break;
	case CTRL_PARAM_DIF_DL:
// GG129000(S) R.Endo 2023/01/26 �Ԕԃ`�P�b�g���X4.1 #6817 �����ݒ�v���ɐݒ肵�����ߗv���������u�Ď��f�[�^�ɐݒ肳��Ȃ��ꍇ������(����Ō��o) [���ʉ��P���� No 1546]
// 		type = PARAM_DL_TIME;
		type = PARAM_DL_DIF_TIME;
// GG129000(E) R.Endo 2023/01/26 �Ԕԃ`�P�b�g���X4.1 #6817 �����ݒ�v���ɐݒ肵�����ߗv���������u�Ď��f�[�^�ɐݒ肳��Ȃ��ꍇ������(����Ō��o) [���ʉ��P���� No 1546]
		break;
	case CTRL_PARAM_UPLOAD:
		type = PARAM_UP_TIME;
		break;
	case CTRL_RESET:
		type = RESET_TIME;
		break;
	case CTRL_PROG_SW:
		type = PROG_ONLY_TIME;
		break;
	case CTRL_CONNECT_CHK:
		type = TEST_TIME;
		break;
	default:
		return 0;
	}
	return chg_info.dl_info[type].From;
}
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
// GG120600(S) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
u_inst_no* remotedl_instNo_get_with_Time_Type(uchar timeType)
{
	if( timeType < TIME_MAX){
		return &chg_info.dl_info[timeType].u;
	}
	// NULL�ŕԂ��̂������Ȃ��߁ATEST�̓��e��ԋp����
	return &chg_info.dl_info[TEST_TIME].u;
}

uchar remotedl_instFrom_get_with_Time_Type(uchar timeType)
{
	if( timeType < TIME_MAX){
		return chg_info.dl_info[timeType].From;
	}
	return 0;
}
// GG120600(E) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)

//[]----------------------------------------------------------------------[]
///	@brief			���u�Ď��f�[�^�o�^
//[]----------------------------------------------------------------------[]
///	@param[in]		Code	: �R�[�h
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void rmon_regist(ulong Code)
{
	u_inst_no *pInstNo;
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
	uchar connect_type;
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

	if (Code == 0) {
		return;
	}
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	// ���g���C���͊Ď��f�[�^��o�^���Ȃ�
//	if (now_retry_active_chk(RETRY_KIND_MAX) || chg_info.retry_dl_cnt) {
//		return;
//	}
	connect_type = remotedl_connect_type_get();
	// CTRL_CONNECT_CHK�́Aretry_dl_cnt���Ȃ��̂ŏ��O
	if(connect_type != CTRL_CONNECT_CHK){
		if(chg_info.retry_dl_cnt[connect_type_to_dl_time_kind(connect_type)]){
			return;
		}
	}
	switch (connect_type) {
	case CTRL_PROG_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PRG) || now_retry_active_chk(RETRY_KIND_DL_PRG)){
			return;
		}
		break;
	case CTRL_PARAM_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL) || now_retry_active_chk(RETRY_KIND_DL_PARAM)){
			return;
		}
		break;
	case CTRL_PARAM_DIF_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL_DIFF) || now_retry_active_chk(RETRY_KIND_DL_PARAM_DIFF)){
			return;
		}
		break;
	case CTRL_PARAM_UPLOAD:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_UP) || now_retry_active_chk(RETRY_KIND_UL)){
			return;
		}
		break;
	case CTRL_RESET:
	case CTRL_PROG_SW:
	case CTRL_CONNECT_CHK:
	default:
		break;
	}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
	if ((pInstNo = remotedl_instNo_get()) != NULL) {
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, remotedl_monitor_info_get(), pInstNo, NULL, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, remotedl_monitor_info_get(), pInstNo, NULL, NULL, NULL,remotedl_instFrom_get(),0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�Ď��f�[�^�o�^(���ߔԍ�����)
//[]----------------------------------------------------------------------[]
///	@param[in]		Code		: �R�[�h
///	@param[in]		*pInstNo	: ���ߔԍ�
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//void rmon_regist_ex(ulong Code, ulong *pInstNo)
void rmon_regist_ex(ulong Code, ulong *pInstNo,uchar from)
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
{
	switch (Code) {
	case RMON_PRG_DL_CHECK_OK:			// [�o�[�W�����A�b�v �\��m�F����]-[��t����]
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PROG_DL_TIME].start_time,
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				&chg_info.sw_info[SW_PROG].sw_time, &chg_info.script[PROGNO_KIND_DL][0]);
				&chg_info.sw_info[SW_PROG].sw_time, &chg_info.script[PROGNO_KIND_DL][0],from,0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	case RMON_PRM_DL_CHECK_OK:			// [�ݒ�ύX(�ݒ�_�E�����[�h) �\��m�F����]-[��t����]
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_DL_TIME].start_time,
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				&chg_info.sw_info[SW_PARAM].sw_time, NULL);
				&chg_info.sw_info[SW_PARAM].sw_time, NULL,from,0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	case RMON_PRM_UP_CHECK_OK:			// [�ݒ�]��(�ݒ�A�b�v���[�h) �\��m�F����]-[��t����]
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_UP_TIME].start_time, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_UP_TIME].start_time, NULL, NULL,from,0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	case RMON_RESET_CHECK_OK:			// [�ċN��(���Z�b�g) �\��m�F����]-[��t����]
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[RESET_TIME].start_time, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[RESET_TIME].start_time, NULL, NULL,from,0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	case RMON_PRG_SW_CHECK_OK:			// [�v���O�����؊� �\��m�F����]-[��t����]
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.sw_info[SW_PROG_EX].sw_time, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.sw_info[SW_PROG_EX].sw_time, NULL, NULL,from,0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
// MH810100(S) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
	case RMON_PRM_DIF_DL_CHECK_OK:		// [�����ݒ�ύX(�ݒ�_�E�����[�h) �\��m�F����]-[��t����]
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, &chg_info.dl_info[PARAM_DL_DIF_TIME].start_time,
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//				&chg_info.sw_info[SW_PARAM].sw_time, NULL);
				&chg_info.sw_info[SW_PARAM_DIFF].sw_time, NULL,from,0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
// MH810100(E) S.Takahashi 2020/05/13 �Ԕԃ`�P�b�g���X #4162 �����ݒ�\��m�F�̃_�E�����[�h�����A�X�V�������擾�ł��Ȃ�
	default:
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, NULL, NULL, NULL);
		wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)pInstNo, NULL, NULL, NULL,from,0);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
		break;
	}
}

// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
// �[�����N�_�̉��u�Ď��f�[�^
void rmon_regist_ex_FromTerminal(ulong Code)
{
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//	u_inst_no inst;
//	inst.ullinstNo = (ulonglong)-1;
//	rmon_regist_ex(Code,&inst.ulinstNo[0]);
	u_inst_no inst;
	inst.ulinstNo[0] = CLK_REC.year%100*100 + CLK_REC.mont%100;
	inst.ulinstNo[0] = inst.ulinstNo[0]*10000 + CLK_REC.date%100*100 + CLK_REC.hour%100;
	inst.ulinstNo[1] = CLK_REC.minu%100;
	inst.ulinstNo[1] = inst.ulinstNo[1]*100000 + NTNET_MODEL_CODE*100 + CPrmSS[S_PAY][2]%100;
	inst.ulinstNo[1] = inst.ulinstNo[1]*100 + g_usRmonSeq%100;
	g_usRmonSeq++;
	if(g_usRmonSeq >= 99){
		g_usRmonSeq = 0;
	}

	wrmonlg(RMON_FUNC_REMOTEDL, Code, 0, (u_inst_no *)&inst.ulinstNo[0], NULL, NULL, NULL,1,1);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
}

// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
void rmon_regist_with_ConectType(ulong Code,uchar connect_type)
{
	u_inst_no *pInstNo;

	if (Code == 0) {
		return;
	}
	if(chg_info.retry_dl_cnt[connect_type]){
		return;
	}
	switch (connect_type) {
	case CTRL_PROG_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PRG) || now_retry_active_chk(RETRY_KIND_DL_PRG)){
			return;
		}
		break;
	case CTRL_PARAM_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL) || now_retry_active_chk(RETRY_KIND_DL_PARAM)){
			return;
		}
		break;
	case CTRL_PARAM_DIF_DL:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_DL_DIFF) || now_retry_active_chk(RETRY_KIND_DL_PARAM_DIFF)){
			return;
		}
		break;
	case CTRL_PARAM_UPLOAD:
		if (now_retry_active_chk(RETRY_KIND_CONNECT_PARAM_UP) || now_retry_active_chk(RETRY_KIND_UL)){
			return;
		}
		break;
	case CTRL_RESET:
	case CTRL_PROG_SW:
	case CTRL_CONNECT_CHK:
	default:
		break;
	}
// GG120600(S) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
//	if ((pInstNo = &chg_info.dl_info[connect_type].u) != NULL) {
//		wrmonlg(RMON_FUNC_REMOTEDL, Code, chg_info.monitor_info, pInstNo, NULL, NULL, NULL,chg_info.dl_info[connect_type].From,0);
//	}
	if ((pInstNo = &chg_info.dl_info[connect_type_to_dl_time_kind(connect_type)].u) != NULL) {
		wrmonlg(RMON_FUNC_REMOTEDL, Code, chg_info.monitor_info, pInstNo, NULL, NULL, NULL,chg_info.dl_info[connect_type_to_dl_time_kind(connect_type)].From,0);
	}
// GG120600(E) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��

}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

// �[�����Őݒ肪�X�V���ꂽ���߁A�\�񂪓����Ă�����L�����Z�����s��
void remotedl_cancel_setting(void)
{

	int check_request;
	uchar connect_type;
	ulong i;
// GG120600(S) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
	uchar result;
// GG120600(E) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���

	for (i = 0; i < 32; i++) { // 32bit
		check_request = 1 << i;
		if (!remotedl_is_accept(check_request)) {
			continue;
		}

		if ((connect_type = remotedl_request_kind_to_connect_type(check_request)) == CTRL_NONE) {
			// �s���ȃr�b�g��ON�ɂȂ��Ă���
			continue;
		}

		switch (connect_type) {
		case CTRL_PARAM_DL:
// GG120600(S) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
// //			retry_info_clr(RETRY_KIND_MAX);
// //			remotedl_start_retry_clear();
// 			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
// 			retry_info_clr(RETRY_KIND_DL_PARAM);
// 			remotedl_start_retry_clear(PARAM_DL_TIME);
// GG120600(S) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			rmon_regist_with_ConectType(RMON_PRM_DL_START_CANCEL,connect_type);			// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)
// GG120600(E) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			remotedl_recv_info_clear(PARAM_DL_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// //			rmon_regist_with_ConectType(RMON_PRM_DL_START_CANCEL,connect_type);			// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)
// GG120600(E) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			// �󂯕t�����v�����N���A����
// 			remotedl_complete_request(REQ_KIND_PARAM_CHG);
// 			// ���u�����e�i���X�����N���A
// 			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));

			// �N���A���Ă��܂��̂ł����Ō��ʎ擾�i�N���A���Ă��牓�u�Ď��f�[�^���Ă΂Ȃ��ƁA���g���C�ő��M����Ȃ��E�E�E�E�j
			result = remotedl_result_get(RES_DL_PARAM);
			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL);
			retry_info_clr(RETRY_KIND_DL_PARAM);
			remotedl_start_retry_clear(PARAM_DL_TIME);
			if( result == PROG_DL_COMP){
				// DL�ς݂Ȃ̂ōX�V�J�n�L�����Z��
				rmon_regist_with_ConectType(RMON_PRM_SW_START_CANCEL,connect_type);			// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)
			}else{
				rmon_regist_with_ConectType(RMON_PRM_DL_START_CANCEL,connect_type);			// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)

			}
			remotedl_recv_info_clear(PARAM_DL_TIME);
			// �󂯕t�����v�����N���A����
			remotedl_complete_request(REQ_KIND_PARAM_CHG);
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_DL_TIME], 0, sizeof(t_remote_time_info));

// GG120600(E) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
			break;
		case CTRL_PARAM_DIF_DL:
// GG120600(S) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
// //			retry_info_clr(RETRY_KIND_MAX);
// //			remotedl_start_retry_clear();
// 			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
// 			retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
// 			remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
// GG120600(S) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			rmon_regist_with_ConectType(RMON_PRM_DL_DIF_START_CANCEL,connect_type);		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX
// GG120600(E) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// //			rmon_regist_with_ConectType(RMON_PRM_DL_DIF_START_CANCEL,connect_type);		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX
// GG120600(E) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			// �󂯕t�����v�����N���A����
// 			remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
// 			// ���u�����e�i���X�����N���A
// 			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));

			// �N���A���Ă��܂��̂ł����Ō��ʎ擾�i�N���A���Ă��牓�u�Ď��f�[�^���Ă΂Ȃ��ƁA���g���C�ő��M����Ȃ��E�E�E�E�j
			result = remotedl_result_get(RES_DL_PARAM_DIF);
			retry_info_clr(RETRY_KIND_CONNECT_PARAM_DL_DIFF);
			retry_info_clr(RETRY_KIND_DL_PARAM_DIFF);
			remotedl_start_retry_clear(PARAM_DL_DIF_TIME);
			if( result == PROG_DL_COMP){
				// DL�ς݂Ȃ̂ōX�V�J�n�L�����Z��
				rmon_regist_with_ConectType(RMON_PRM_SW_DIF_START_CANCEL,connect_type);		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX
			}else{
				rmon_regist_with_ConectType(RMON_PRM_DL_DIF_START_CANCEL,connect_type);		// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX
			}
			remotedl_recv_info_clear(PARAM_DL_DIF_TIME);
			// �󂯕t�����v�����N���A����
			remotedl_complete_request(REQ_KIND_PARAM_DIF_CHG);
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_DL_DIF_TIME], 0, sizeof(t_remote_time_info));
// GG120600(E) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
			break;
		case CTRL_PARAM_UPLOAD:
// GG120600(S) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
// //			retry_info_clr(RETRY_KIND_MAX);
// //			remotedl_start_retry_clear();
// 			retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
// 			retry_info_clr(RETRY_KIND_UL);
// 			remotedl_start_retry_clear(PARAM_UP_TIME);
// GG120600(S) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			rmon_regist_with_ConectType(RMON_PRM_UP_START_CANCEL,connect_type);			// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX
// GG120600(E) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			remotedl_recv_info_clear(PARAM_UP_TIME);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
// GG120600(S) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// //			rmon_regist_with_CooectType(RMON_PRM_UP_START_CANCEL,connect_type);			// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX
// GG120600(E) // Phase9 #5069 [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX)�̖��ߔԍ����s��
// 			// �󂯕t�����v�����N���A����
// 			remotedl_complete_request(REQ_KIND_PARAM_UL);
// 			// ���u�����e�i���X�����N���A
// 			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));

			// UP�͍X�V���Ȃ��̂ł킯�Ȃ�
			retry_info_clr(RETRY_KIND_CONNECT_PARAM_UP);
			retry_info_clr(RETRY_KIND_UL);
			remotedl_start_retry_clear(PARAM_UP_TIME);
			rmon_regist_with_ConectType(RMON_PRM_UP_START_CANCEL,connect_type);			// [�L�����Z��]�i�[���Őݒ�p�����[�^�ύX
			remotedl_recv_info_clear(PARAM_UP_TIME);
			// �󂯕t�����v�����N���A����
			remotedl_complete_request(REQ_KIND_PARAM_UL);
			// ���u�����e�i���X�����N���A
			memset(&remote_dl_info.time_info[PARAM_UP_TIME], 0, sizeof(t_remote_time_info));

// GG120600(E) // Phase9 #5078 �y�A���]���w�E�����z�[�����Ńp�����[�^�ύX��ɑ��M����鉓�u�Ď��f�[�^�̏����ԍ����Ԉ���Ă���
			break;
		}
	}
	return;

}
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�


//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�������擾
//[]----------------------------------------------------------------------[]
///	@param[in]		type	: �v�����
///	@return			ret		: �������|�C���^
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
t_remote_time_info* remotedl_time_info_get(uchar type)
{
	t_remote_time_info *pTime = NULL;

	if (type < TIME_INFO_MAX) {
		pTime = &remote_dl_info.time_info[type];
	}

	return pTime;
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�������N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �v�����
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//void remotedl_time_info_clear(uchar type)
void remotedl_time_info_clear(uchar type,u_inst_no *pInstNo,uchar from)
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
{
	uchar ReqKind;

	switch (type) {
	case PROG_DL_TIME:
		ReqKind = VER_UP_REQ;
		break;
	case PARAM_DL_TIME:
		ReqKind = PARAM_CHG_REQ;
		break;
	case PARAM_UP_TIME:
		ReqKind = PARAM_UPLOAD_REQ;
		break;
	case RESET_TIME:
		ReqKind = RESET_REQ;
		break;
	case PROG_ONLY_TIME:
		ReqKind = PROG_ONLY_CHG_REQ;
		break;
// MH810100(S) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)		
	case PARAM_DL_DIF_TIME:
		ReqKind = PARAM_DIFF_CHG_REQ;
		break;
// MH810100(E) Y.Yamauchi 2019/12/23 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)		
	default:
		return;
	}

	// �v�����
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
////	remotedl_proc_cancel(ReqKind, &chg_info.dl_info[type].u.ulinstNo[0], TRUE);
//	remotedl_proc_cancel(ReqKind, &chg_info.dl_info[type].u.ulinstNo[0], TRUE,chg_info.dl_info[type].From);
//// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
	remotedl_proc_cancel(ReqKind, &pInstNo->ulinstNo[0], TRUE,from);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
}

// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
////[]----------------------------------------------------------------------[]
/////	@brief			���u�����e�i���X�������S�N���A
////[]----------------------------------------------------------------------[]
/////	@param[in]		void
/////	@return			void
/////	@attention		None
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2015-04-08<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
//void remotedl_time_info_clear_all(void)
//{
//	uchar type;
//
//	for (type = PROG_DL_TIME; type < TIME_MAX; type++) {
//		remotedl_time_info_clear(type);
//	}
//}
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�������ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �������
///	@param[in]		type	: �v�����
///	@param[in]		*pTime	: �������|�C���^
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_time_set(uchar kind, uchar type, date_time_rec *pTime)
{
	switch (kind) {
	case INFO_KIND_START:
		if (type < TIME_INFO_MAX) {
			// �J�n�������Z�b�g
			memcpy(&remote_dl_info.time_info[type].start_time, pTime, sizeof(*pTime));
			// �����X�e�[�^�X���N���A
			remote_dl_info.time_info[type].status[INFO_KIND_START] = EXEC_STS_NONE;
		}
		break;
	case INFO_KIND_SW:
		if (type < TIME_INFO_MAX) {
			// �X�V�������Z�b�g
			memcpy(&remote_dl_info.time_info[type].sw_time, pTime, sizeof(*pTime));
			// �����X�e�[�^�X���N���A
			remote_dl_info.time_info[type].status[INFO_KIND_SW] = EXEC_STS_NONE;
		}
		break;
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���u�����e�i���X�����X�e�[�^�X�ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �������
///	@param[in]		type	: �v�����
///	@param[in]		status	: �����X�e�[�^�X
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_comp_set(uchar kind, uchar type, uchar status)
{
	if ((kind < INFO_KIND_MAX) && (type < TIME_INFO_MAX)) {
		// �X�e�[�^�X���Z�b�g
		remote_dl_info.time_info[type].status[kind] = status;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP���ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		*pFtpInfo	: FTP���|�C���^
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_ftp_info_set(t_FtpInfo *pFtpInfo)
{
	memcpy(&g_bk_FtpInfo, pFtpInfo, sizeof(*pFtpInfo));
}

//[]----------------------------------------------------------------------[]
///	@brief			FTP���擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: FTP���|�C���^
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
t_FtpInfo* remotedl_ftp_info_get(void)
{
	return &g_bk_FtpInfo;
}

//[]----------------------------------------------------------------------[]
///	@brief			�����M�f�[�^�����擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: �����M�f�[�^����
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ulong remotedl_unsent_data_count_get(void)
{
	int i,j;
	ulong data_count = 0;
	ulong *pBufCnt;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	NTBUF_GetBufCount(&g_bufcnt);
	NTBUF_GetBufCount(&g_bufcnt, TRUE);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	pBufCnt = (ulong *)&g_bufcnt;

	for (i = 0; i <= 1; i++) {
		for (j = 6; j > 0; j--) {
			// ���M�}�X�N�ݒ�`�F�b�N
			if (prm_get(COM_PRM, S_NTN, 61+i, 1, j) == 0) {
				// �����`���̏ꍇ
				if (prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) {
					// �ޑK�Ǘ��W�v�f�[�^�̓J�E���g���Ȃ�
					if ((i == 1) && (j == 2)) {
						continue;
					}
				}
				// �W�v�f�[�^�̏ꍇ�A�T�[�o�^�C�v���̃f�[�^���M�p�����[�^���`�F�b�N
				if ((i == 0) && (j == 4) && (0 == prm_get(COM_PRM, S_NTN, 26, 1, 3))) {
					continue;
				}
				data_count += *(pBufCnt + st_offset[i][j-1]);
			}
		}
	}
	// ����A���u�Ď��f�[�^�̑��M�}�X�N�ݒ肪�Ȃ��̂ŃZ���^�[�`���ł���΃J�E���g����
	if (prm_get(COM_PRM, S_NTN, 121, 1, 1) == 1) {
		data_count += g_bufcnt.sndbuf_rmon;
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
		data_count += g_bufcnt.sndbuf_lpark;// �������Ԃ��ݒ�}�X�N�͂Ȃ��̂ŃZ���^�[�`���ł͉��Z�Ώ�
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	}
	return data_count;
}

//[]----------------------------------------------------------------------[]
///	@brief			�v���O��������+�o�[�W�����擾
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*pProgNo	: ������o�b�t�@
///	@param[in]		size		: ������o�b�t�@�T�C�Y
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_prog_no_get(char *pProgNo, int size)
{
	if (pProgNo && (size >= sizeof(chg_info.script))) {
		strncpy(pProgNo, (char *)chg_info.script, sizeof(chg_info.script));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�v���O���������ݏ��ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		*pInfo	: �v���O���������ݏ��|�C���^
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_write_info_set(t_write_info *pInfo)
{
	if (pInfo) {
		memcpy(&chg_info.write_info, pInfo, sizeof(chg_info.write_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�v���O���������ݏ��擾
//[]----------------------------------------------------------------------[]
///	@param[in/out]	*pInfo	: �v���O���������ݏ��|�C���^
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_write_info_get(t_write_info *pInfo)
{
	if (pInfo) {
		memcpy(pInfo, &chg_info.write_info, sizeof(chg_info.write_info));
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�v���O�����t�@�C�������ݏ��N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_write_info_clear(void)
{
	memset(&chg_info.write_info, 0, sizeof(chg_info.write_info));
}

//[]----------------------------------------------------------------------[]
///	@brief			�v����t�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		ReqKind	: �v�����
///	@return			ret		: 0=��t�s��, 1=��t, -1=�ΏۊO
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
int remotedl_check_request(uchar ReqKind)
{
	int ret = -1;

	if (0 < ReqKind && ReqKind < REMOTE_REQ_MAX) {
		// �v���`�F�b�N
		if (ReqAcceptTbl[ReqKind]) {
			ret = 1;
		}
		else {
			ret = 0;
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�[���Ď����ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		info	: �[���Ď����
///	@return			None
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_monitor_info_set(ushort info)
{
	chg_info.monitor_info = info;
}

//[]----------------------------------------------------------------------[]
///	@brief			�[���Ď����擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: �[���Ď����
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-03-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
ushort remotedl_monitor_info_get(void)
{
	return chg_info.monitor_info;
}

//[]----------------------------------------------------------------------[]
///	@brief			���d�t���O�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param[in]		flg 	: ���d�t���O
///	@return			void
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2015-04-14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2015 AMANO Corp.---[]
void remotedl_pow_flg_set(BOOL bFlg)
{
	chg_info.pow_flg = bFlg;
}

// MH810103(s) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��
//// MH322915(S) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
////[]----------------------------------------------------------------------[]
//// 		���ԃv���t�B�b�N�X�擾
////[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
//static eVER_TYPE GetVersionPrefix( uchar *buf, ushort size )
//{
//	const char pref[VER_TYPE_MAX][5] = { "MH", "GM", "GW", "GG" };
//	eVER_TYPE type = VER_TYPE_MAX;
//	ushort i;
//
//	if( buf == NULL || size < 2 ){
//		return type;
//	}
//	for( i=0; i<VER_TYPE_MAX; i++ ){
//		if( strncmp( (char*)buf, pref[i], 2 ) == 0 ){
//			type = (eVER_TYPE)i;
//			break;
//		}
//	}
//	return type;
//}
//// MH322915(E) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
// MH810103(e) �d�q�}�l�[�Ή� ���ԃ`�F�b�N���O��

// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�v����ʁ�������ʕϊ�
//[]----------------------------------------------------------------------[]
///	@param[in]		request	: �v�����
///	@return			�������
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static uchar remotedl_request_kind_to_connect_type(int request)
{
	uchar connect_type;

	switch (request) {
		case REQ_KIND_VER_UP:
			connect_type = CTRL_PROG_DL;
			break;
		case REQ_KIND_PARAM_CHG:
			connect_type = CTRL_PARAM_DL;
			break;
		case REQ_KIND_PARAM_UL:
			connect_type = CTRL_PARAM_UPLOAD;
			break;
		case REQ_KIND_RESET:
			connect_type = CTRL_RESET;
			break;
		case REQ_KIND_PROG_ONLY_CHG:
			connect_type = CTRL_PROG_SW;
			break;
		case REQ_KIND_FTP_CHG:
			connect_type = CTRL_NONE;
			break;
		case REQ_KIND_TEST:
			connect_type = CTRL_CONNECT_CHK;
			break;
		case REQ_KIND_PARAM_DIF_CHG:
			connect_type = CTRL_PARAM_DIF_DL;
			break;
		case REQ_KIND_PROGNO_CHG:
		default:
			connect_type = CTRL_NONE;
			break;
	}

	return connect_type;
}

//[]----------------------------------------------------------------------[]
///	@brief			���̏�����ʂ�I��
//[]----------------------------------------------------------------------[]
///	@param[in]		���u�����e�i���X���
///	@return			�ŒZ���s����
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static ulong remotedl_connect_type_set_next(t_prog_chg_info *pg_info)
{
	ulong next_time = 0xFFFFFFFF, check_time;
	int check_request;
	uchar connect_type, next_connect_type = CTRL_NONE;
	uchar status, next_status = R_DL_IDLE;
	ulong i;
	date_time_rec *target_time;
// GG120600(S) // Phase9 ���ʂ���ʂ���
	uchar result;
// GG120600(E) // Phase9 ���ʂ���ʂ���
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
	uchar kind_connect;
	uchar kind_updown;
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

	if (pg_info->req_accept != 0) {
		for (i = 0; i < 32; i++) { // 32bit
			if (next_connect_type == CTRL_CONNECT_CHK) {
				// FTP�ڑ��e�X�g�͍ŗD��
				break;
			}

			check_request = 1 << i;
			if (!remotedl_is_accept(check_request)) {
				continue;
			}

			if ((connect_type = remotedl_request_kind_to_connect_type(check_request)) == CTRL_NONE) {
				// �s���ȃr�b�g��ON�ɂȂ��Ă���
				continue;
			}
// GG120600(S) // Phase9 ���ʂ���ʂ���
			switch (connect_type) {
				case CTRL_PROG_DL:
					result = pg_info->result[RES_DL_PROG];
					kind_connect = RETRY_KIND_CONNECT_PRG;
					kind_updown = RETRY_KIND_DL_PRG;
					break;
				case CTRL_PARAM_DL:
					result = pg_info->result[RES_DL_PARAM];
					kind_connect = RETRY_KIND_CONNECT_PARAM_DL;
					kind_updown = RETRY_KIND_DL_PARAM;
					break;
				case CTRL_PARAM_DIF_DL:
					result = pg_info->result[RES_DL_PARAM_DIF];
					kind_connect = RETRY_KIND_CONNECT_PARAM_DL_DIFF;
					kind_updown = RETRY_KIND_DL_PARAM_DIFF;
					break;
				case CTRL_PARAM_UPLOAD:
					kind_connect = RETRY_KIND_CONNECT_PARAM_UP;
					kind_updown = RETRY_KIND_UL;
					break;
			}
// GG120600(E) // Phase9 ���ʂ���ʂ���

			switch (connect_type) {
				case CTRL_PROG_DL:
				case CTRL_PARAM_DL:
				case CTRL_PARAM_DIF_DL:
					status = R_DL_REQ_RCV;
// GG120600(S) // Phase9 ���ʂ���ʂ���
//					if (pg_info->result[RES_DL] != EXCLUDED) {
					if (result == PROG_DL_COMP) {
// GG120600(E) // Phase9 ���ʂ���ʂ���
//						status = R_SW_WAIT;
						switch (connect_type) {
							case CTRL_PROG_DL:
								if (pg_info->sw_info[SW_PROG].exec != 0) {
									status = R_SW_WAIT;
									target_time = &pg_info->sw_info[SW_PROG].sw_time;
								}
								else {
									// �[���Ƃ̓]���� or �[���ł̍X�V���̓X�e�[�^�X���X�V���Ȃ�
									continue;
								}
								break;
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
//							case CTRL_PARAM_DL:
//							case CTRL_PARAM_DIF_DL:
//								if (pg_info->sw_info[SW_PARAM].exec != 0) {
//									status = R_SW_WAIT;
//									target_time = &pg_info->sw_info[SW_PARAM].sw_time;
//								}
//								else {
//									// �[���ւ̓]���� or �[���ł̍X�V���̓X�e�[�^�X���X�V���Ȃ�
//									continue;
//								}
//								break;
							case CTRL_PARAM_DL:
								if (pg_info->sw_info[SW_PARAM].exec != 0) {
									status = R_SW_WAIT;
									target_time = &pg_info->sw_info[SW_PARAM].sw_time;
								}
								else {
									// �[���ւ̓]���� or �[���ł̍X�V���̓X�e�[�^�X���X�V���Ȃ�
									continue;
								}
								break;
							case CTRL_PARAM_DIF_DL:
								if (pg_info->sw_info[SW_PARAM_DIFF].exec != 0) {
									status = R_SW_WAIT;
									target_time = &pg_info->sw_info[SW_PARAM_DIFF].sw_time;
								}
								else {
									// �[���ւ̓]���� or �[���ł̍X�V���̓X�e�[�^�X���X�V���Ȃ�
									continue;
								}
								break;
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
						}
						check_time = Nrm_YMDHM(target_time);
					}
// GG120600(S) // Phase9 ���ʂ���ʂ���
//					else if (now_retry_active_chk( RETRY_KIND_DL)) {
//						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_DL].time);
					else if (now_retry_active_chk( kind_updown)) {
						check_time = Nrm_YMDHM(&pg_info->retry[kind_updown].time);
// GG120600(E) // Phase9 ���ʂ���ʂ���
						status = R_DL_IDLE;
					}
// GG120600(S) // Phase9 ���ʂ���ʂ���
//					else if (now_retry_active_chk( RETRY_KIND_CONNECT)) {
//						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_CONNECT].time);
					else if (now_retry_active_chk( kind_connect)) {
						check_time = Nrm_YMDHM(&pg_info->retry[kind_connect].time);
// GG120600(E) // Phase9 ���ʂ���ʂ���
						status = R_DL_IDLE;
					}
					else {
						switch (connect_type) {
							case CTRL_PROG_DL:
								target_time = &pg_info->dl_info[PROG_DL_TIME].start_time;
								break;
							case CTRL_PARAM_DL:
								target_time = &pg_info->dl_info[PARAM_DL_TIME].start_time;
								break;
							case CTRL_PARAM_DIF_DL:
								target_time = &pg_info->dl_info[PARAM_DL_DIF_TIME].start_time;
								break;
						}
						check_time = Nrm_YMDHM(target_time);
					}
					break;
				case CTRL_PARAM_UPLOAD:
					status = R_UP_WAIT;
					if (now_retry_active_chk( RETRY_KIND_UL)) {
						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_UL].time);
						status = R_DL_IDLE;
					}
// GG120600(S) // Phase9 ���ʂ���ʂ���
//					else if (now_retry_active_chk( RETRY_KIND_CONNECT)) {
//						check_time = Nrm_YMDHM(&pg_info->retry[RETRY_KIND_CONNECT].time);
					else if (now_retry_active_chk( kind_connect)) {
						check_time = Nrm_YMDHM(&pg_info->retry[kind_connect].time);
// GG120600(E) // Phase9 ���ʂ���ʂ���
						status = R_DL_IDLE;
					}
					else {
						check_time = Nrm_YMDHM(&pg_info->dl_info[PARAM_UP_TIME].start_time);
					}
					break;
				case CTRL_RESET:
					status = R_RESET_WAIT;
					check_time = Nrm_YMDHM(&pg_info->dl_info[RESET_TIME].start_time);
					break;
				case CTRL_PROG_SW:
					status = R_SW_WAIT;
					check_time = Nrm_YMDHM(&pg_info->sw_info[SW_PROG_EX].sw_time);
					break;
				case CTRL_CONNECT_CHK:
					status = R_TEST_CONNECT;
					check_time = 0;
					break;
			}

			if (check_time < next_time) {
				next_status = status;
				next_time = check_time;
				next_connect_type = connect_type;
			}
		}
	}

	remotedl_connect_type_set(next_connect_type);
	remotedl_status_set(next_status);
	return next_time;
}
//[]----------------------------------------------------------------------[]
///	@brief			���̏�����ʂ�I��
//[]----------------------------------------------------------------------[]
///	@param[in]		None
///	@return			None
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void remotedl_arrange_next_request(void)
{
	// ulong next_time = 0xFFFFFFFF, check_time;
	// ulong i;
	// uchar next_status = R_DL_IDLE;

	// for (i = 0; i < sizeof(chg_info) / sizeof(chg_info[0]); i++) {
	// 	check_time = remotedl_connect_type_set_next(&chg_info[i]);
	// 	if (check_time < next_time) {
	// 		next_status = remotedl_status_get();
	// 	}
	// }
	// remotedl_status_set(next_status);
	uchar next_status = R_DL_IDLE;

	remotedl_connect_type_set_next(&chg_info);
	next_status = remotedl_status_get();
	remotedl_status_set(next_status);
}
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
// GG120600(S) // Phase9 CRC�`�F�b�N
void	remotedl_prog_crc_set( uchar uc1,uchar uc2)
{															
	chg_info.CRC16.crc[0] = uc1;
	chg_info.CRC16.crc[1] = uc2;
}															
ushort	remotedl_prog_crc_get( void )			
{															
	return	(chg_info.CRC16.uscrc);
}															
// GG120600(E) // Phase9 CRC�`�F�b�N

// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//[]----------------------------------------------------------------------[]
///	@brief			������ʁ��_�E�����[�h���Ԏ�ʕϊ�
//[]----------------------------------------------------------------------[]
///	@param[in]		connect_type	:	�������
///	@return			�_�E�����[�h���Ԏ��
///	@attention		None
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static uchar connect_type_to_dl_time_kind(uchar connect_type)
{
	uchar time_kind = TIME_MAX;

	switch (connect_type) {
		case CTRL_PROG_DL:
			time_kind = PROG_DL_TIME;
			break;
		case CTRL_PARAM_DL:
			time_kind = PARAM_DL_TIME;
			break;
		case CTRL_PARAM_UPLOAD:
			time_kind = PARAM_UP_TIME;
			break;
		case CTRL_RESET:
			time_kind = RESET_TIME;
			break;
		case CTRL_PROG_SW:
			time_kind = PROG_ONLY_TIME;
			break;
		case CTRL_CONNECT_CHK:
			time_kind = TEST_TIME;
			break;
		case CTRL_PARAM_DIF_DL:
			time_kind = PARAM_DL_DIF_TIME;
			break;
	}

	return time_kind;
}
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���

// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���

// chg_info�̓o�b�N�A�b�v��(remotedl_restore�Ńo�b�N�A�b�v���烊�X�g�A�����i�o�b�N�A�b�v�͍ċN���O�Ɏ��{����Ă���)
// remote_dl_info�́A�o�b�N�A�b�v�łȂ��ق��ցi�N��������remotedl_update_chk�Ńo�b�N�A�b�v�����j

void remotedl_chg_info_log0_to1(void)
{
	int i;
	t_prog_chg_info_V0 chg_info_v0;
	t_remote_dl_info_V0 dl_info_v0;
	uchar* base = (uchar*)&chg_info;

	// V0���̈ʒu���Z�o
	t_prog_chg_info_V0* pbk = (t_prog_chg_info_V0*)(base + sizeof(t_prog_chg_info_V0));
	t_remote_dl_info_V0* pinfo = (t_remote_dl_info_V0*)(base + sizeof(t_prog_chg_info_V0) + sizeof(t_prog_chg_info_V0));
	// ��U�R�s�[
	memcpy(&chg_info_v0, pbk,sizeof(t_prog_chg_info_V0));
	memcpy(&dl_info_v0, pinfo,sizeof(t_remote_dl_info_V0));
	// ��U�N���A
	memset(&bk_chg_info,0,sizeof(t_prog_chg_info));
	memset(&remote_dl_info,0,sizeof(t_remote_dl_info));
	//////////////////////////////////////
	// chg_info�̓o�b�N�A�b�v��
	//////////////////////////////////////
	bk_chg_info.req_accept = chg_info_v0.req_accept;
	bk_chg_info.connect_type = chg_info_v0.connect_type;
	bk_chg_info.status = chg_info_v0.status;
	bk_chg_info.script_type = chg_info_v0.script_type;
	bk_chg_info.update = chg_info_v0.update;
	memcpy(bk_chg_info.result,chg_info_v0.result,sizeof(uchar)*RES_MAX_V0);

	memset(bk_chg_info.retry_dl_cnt,0, sizeof(ushort)*TIME_INFO_MAX);	// // �ċN���Ȃ̂�0�Œ�

	bk_chg_info.failsafe_timer = chg_info_v0.failsafe_timer;
	bk_chg_info.monitor_info = chg_info_v0.monitor_info;
	bk_chg_info.param_up = chg_info_v0.param_up;
	bk_chg_info.exec_info = chg_info_v0.exec_info;
	memset(bk_chg_info.retry,0, sizeof(t_retry_info)*RETRY_KIND_MAX_V0);	// // �ċN���Ȃ̂�0�Œ�

	// diff���͕s�v�Ȃ̂ł��̂܂�
	memcpy(bk_chg_info.sw_status,chg_info_v0.sw_status,sizeof(uchar)*(SW_MAX_V0-1));

	bk_chg_info.pow_flg = chg_info_v0.pow_flg;
// -----------------------------------�� ���s�����
// -----------------------------------�� ��M���
	// �ڑ��e�X�g�O�܂�COPY
	for(i = 0; i < TIME_MAX_V0 - 1;i++){
		// �v�������΂��ăR�s�[���Ă���
		memcpy(&bk_chg_info.dl_info[i].u ,&chg_info_v0.dl_info[i].u,sizeof(u_inst_no));
		memcpy(&bk_chg_info.dl_info[i].start_time ,&chg_info_v0.dl_info[i].start_time,sizeof(date_time_rec));
		memcpy(&bk_chg_info.dl_info[i].ftpaddr ,&chg_info_v0.dl_info[i].ftpaddr,sizeof(u_ipaddr));
		bk_chg_info.dl_info[i].ftpport = chg_info_v0.dl_info[i].ftpport;
		memcpy(bk_chg_info.dl_info[i].ftpuser ,chg_info_v0.dl_info[i].ftpuser,sizeof(chg_info_v0.dl_info[i].ftpuser));
		memcpy(bk_chg_info.dl_info[i].ftppasswd ,chg_info_v0.dl_info[i].ftppasswd,sizeof(chg_info_v0.dl_info[i].ftppasswd));
	}
	// �����O�܂ł�COPY
	memcpy(bk_chg_info.sw_info,chg_info_v0.sw_info,sizeof(t_sw_info)*SW_MAX_V0);

	memcpy(bk_chg_info.script,chg_info_v0.script,sizeof(chg_info_v0.script));
	memcpy(&bk_chg_info.write_info,&chg_info_v0.write_info,sizeof(t_write_info));

	//////////////////////////////////////
	// remote_dl_info�́A�o�b�N�A�b�v�łȂ��ق���
	//////////////////////////////////////
	memcpy(&remote_dl_info,&dl_info_v0, sizeof(t_remote_time_info)*TIME_INFO_MAX_V0);

}
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
// GG120600(S) // Phase9 LZ122603(S) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
static long remotedl_nrm_time_sub(ulong from, ulong to)
{
	t_NrmYMDHM _from, _to;
	ulong tmp1, tmp2;
	long tmp3, tmp4;
	long diff, min_diff;
	BOOL borrow = FALSE, positive = from <= to;

	_from.ul = from;
	_to.ul = to;

	if (positive) {
		tmp1 = (ulong)_from.us[1];
		tmp2 = (ulong)_to.us[1];
	}
	else {
		tmp1 = (ulong)_to.us[1];
		tmp2 = (ulong)_from.us[1];
	}
	tmp3 = (long)tmp1;
	tmp4 = (long)tmp2;
	if (tmp3 > tmp4) {
		borrow = TRUE;
		tmp4 += 1440;
	}
	min_diff = tmp4 - tmp3;

	if (positive) {
		tmp1 = (ulong)_from.us[0];
		tmp2 = (ulong)_to.us[0];
	}
	else {
		tmp1 = (ulong)_to.us[0];
		tmp2 = (ulong)_from.us[0];
	}
	tmp3 = (long)tmp1;
	tmp4 = (long)tmp2;

	diff = (tmp4 - tmp3) * 1440;
	if (borrow) {
		diff -= 1440;
	}
	diff += min_diff;

	if (!positive) {
		diff *= -1;
	}

	return diff;
}
// GG120600(E) // Phase9 LZ122603(E) ParkingWeb(�t�F�[�Y9) (#5821:�����ׂ��Ɖ��u�����e�i���X�v�������s����Ȃ��Ȃ�)
