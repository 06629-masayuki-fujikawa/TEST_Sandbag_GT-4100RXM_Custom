/*[]----------------------------------------------------------------------[]*/
/*| Mifare�֘A����                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"iodefine.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"mif.h"
#include	"prm_tbl.h"
#include	"lcd_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"

static	uchar	MIF_IsMntReq;

static short Ope_GT_Settei_Check_For_Mif( uchar gt_flg, ushort pno_syu );

/*[]----------------------------------------------------------------------[]*/
/*| Mifare���䏈��                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpMif_ctl( ret )                                        |*/
/*| PARAMETER    : msg  : Message                                          |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpMif_ctl( ushort msg )
{
	uchar	cmd, sts;

	cmd = 0;														// ����ޑ��M�w�� 0:�����M
	sts = 0;														// ����޽ð��

	switch( msg ){
	case IBK_MIF_EVT_ERR:											// �ʐM�ُ�
		cmd = MIF_mod;												// �ް��đ�
		break;

	case IBK_MIF_A2_NG_EVT:											// MIF:�����ް��Ǐo��NG
		switch( MIF_ENDSTS.sts1 ){
		case 0x01:													// �����ID�װ
		case 0x02:													// ����ԍ�NG
		case 0x03:													// ��ۯ����w��NG
		case 0x04:													// �������ް�����
		case 0x05:													// ���w��NG
		case 0x10:													// ���ޖ����m
		case 0x11:													// �Ǐo����
		case 0x12:													// �����ݒ�
		case 0x13:													// KEY�o�^��
		case 0x14:													// ������
		case 0x15:													// �����I���҂�
		case 0x16:													// �F�ط��װ
			break;
		case 0x20:													// Limit Time Over
		case 0x21:													// NAK Retry Over
		case 0x22:													// Time Out
		case 0x23:													// Bcc Err
		case 0x30:													// ���ޖ���
		case 0x40:													// �ݼݸ�NG
		case 0x50:													// Read Verify NG
		case 0x60:													// Write Verify NG
		case 0xf0:													// ���̑��̴װ
			if( !MIF_IsMntReq && (OPECTL.Ope_mod != 2)){
				if( MIF_mod == 0xA2 ){								// ���޾ݽ&�ް��Ǐo�������(A2)���M�ς�?
					cmd = 0xA2;										// ���޾ݽ&�ް��Ǐo�������(A2)�đ�
				}
			}
			break;
		}
		break;

	case IBK_MIF_A3_NG_EVT:											// �������~NG
	case IBK_MIF_A3_OK_EVT:											// �������~OK
		MIF_mod = 0;												// ����ޏ��(�����)
		break;

	case IBK_MIF_A4_OK_EVT:											// MIF:�����ް�������OK
		if( !MIF_IsMntReq ){
			if( OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3 ){
				PassExitTimeTblDelete( MifCard.pk_no, MifCard.pas_id );
			}
		}
		if( MIF_mod == 0xA4 ){										// ������(A4)���M�ς�?
			MIF_mod = 0xA2;											// ����ޏ�Ԃ��޾ݽ&�ް��Ǐo��(A2)��ԂƂ���
		}
		break;

	case IBK_MIF_A4_NG_EVT:											// MIF:�����ް�������NG
		if( !MIF_IsMntReq ){
			if( OPECTL.Ope_mod == 3 ){		//���Z�������̂ݓo�^
				PassExitTimeTblWrite( MifCard.pk_no, MifCard.pas_id, &MifCard.ext_tm );
			}
		}
		switch( MIF_ENDSTS.sts1 ){
		case 0x60:													// Write Verify NG
			err_chk( ERRMDL_MIFARE, ERR_MIF_WRT_VERIFY, 2, 0, 1 );
			cmd = 0xA4;												// ������(A4)�đ�
			break;
		default:													// Write NG(Verify�ȊO)
			err_chk( ERRMDL_MIFARE, ERR_MIF_WRT_NG, 2, 0, 1 );
			break;
		}
		break;

	default:
		break;
	}
	/*--------------*/
	/* ����ޑ��M	*/
	/*--------------*/
	if( cmd ){
		OpMif_snd( cmd, sts );
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare����ޑ��M����                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpMif_snd( cmdid, stat )                                |*/
/*| PARAMETER    : cmdid = ���M��������ID                                 |*/
/*|                stat  = �ð��(ýĺ���ނ݂̂Ŏg�p)                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpMif_snd( uchar cmdid, uchar stat )
{
	MIF_IsMntReq = 0;
		MIF_mod = cmdid;											// ����ޏ�ԍX�V
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare����ޑ��M����(for ������s���)                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpMif_snd2( cmdid, stat )                               |*/
/*| PARAMETER    : cmdid = ���M��������ID                                 |*/
/*|                stat  = �ð��(ýĺ���ނ݂̂Ŏg�p)                       |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpMif_snd2( uchar cmdid, uchar stat )
{
	CMN_UNUSED_PARAMETER(stat);
	MIF_IsMntReq = 1;
		MIF_mod = cmdid;											// ����ޏ�ԍX�V
	if( cmdid == 0xA2 ){
		stat = 1;		// ���m�҂����Ԗ�����
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifareذ��                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MifareDataChk( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ret :  0 = OK                                           |*/
/*|                       1 = ���ԏꇂ�װ                                  |*/
/*|                       2 = �ް��ُ�                                     |*/
/*|                       3 = �����؂�                                     |*/
/*|                       4 = ���o�ɴװ                                    |*/
/*|                       5 = �������                                     |*/
/*|                       6 = �����O                                       |*/
/*|                       9 = ��d�g�p�װ                                  |*/
/*|                      13 = ��ʋK��O                                   |*/
/*|                      25 = �Ԏ�װ                                      |*/
/*|                      26 = �ݒ�װ                                      |*/
/*|                      27 = ������ʴװ                                  |*/
/*|                      29 = �⍇���װ                                    |*/
/*|                      30 = ���Z�����װ                                  |*/
/*|                      99 = HOST�֖⍇��                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const char mgtype[4] = {6,1,2,3};
short	MifareDataChk( void )
{
	short	ret;
	short	wk;
	ushort	us_sday;
	ushort	us_eday;
	short	w_syasyu;
	date_time_rec	extim;
	short	s_ptbl;
	ushort	us_psts[3];
	char	c_prm;
	char	c_ptyp;
	ulong	ul_AlmPrm;
	uchar	uc_GtAlmPrm[10];
	short	data_adr;		// �g�p�\������ʂ��ް����ڽ
	char	data_pos;		// �g�p�\������ʂ��ް��ʒu
	uchar	KigenCheckResult;

	int		i;
	t_MIF_CARDFORM	WK_MifareCard;
	char	GT_Mifare_RCRC_NG = 0;						// 0:�A���`�p�XON  1:�A���`�p�XOFF
	ret = 0;

	if( 0 == OPECTL.ChkPassSyu ){	// Mifare�J�[�h�f�[�^��M
		if(ret == 0){
			wk  = Mifare_WrtNgDataSearch( &MIF_CARD_SID[0] );
			if( 0xffff != wk ){	// ����
				// �x�[�X��Read���̃f�[�^�Ƃ���B
				memcpy( &WK_MifareCard, &MIF_WriteNgCard_ReadData.data[wk].Data, sizeof(t_MIF_CARDFORM) );
				// �L���I���N�����Ŕ�r
				us_sday = dnrmlzm( (short)MIF_WriteNgCard_ReadData.data[wk].Data.limit_e.Year,
								 (short)MIF_WriteNgCard_ReadData.data[wk].Data.limit_e.Mon,
								 (short)MIF_WriteNgCard_ReadData.data[wk].Data.limit_e.Day );	// �o�b�t�@�L���I�������

				us_eday = dnrmlzm( (short)MifCard.limit_e.Year, (short)MifCard.limit_e.Mon, (short)MifCard.limit_e.Day );	// �J�[�h�L���I�������

				if(us_sday == us_eday){
					WK_MifareCard.limit_e = MIF_WriteNgCard_ReadData.data[wk].Data.limit_e;		// �J�[�h�L���I���N����
					WK_MifareCard.kosin_ymd = MIF_WriteNgCard_ReadData.data[wk].Data.kosin_ymd;	// �X�V�N����
				}else if(us_sday < us_eday){
					WK_MifareCard.limit_e = MifCard.limit_e;		// �J�[�h�L���I���N����
					WK_MifareCard.kosin_ymd = MifCard.kosin_ymd;	// �X�V�N����
				}else{
					WK_MifareCard.limit_e = MIF_WriteNgCard_ReadData.data[wk].Data.limit_e;		// �J�[�h�L���I���N����
					WK_MifareCard.kosin_ymd = MIF_WriteNgCard_ReadData.data[wk].Data.kosin_ymd;	// �X�V�N����
				}
				
				memcpy( &MifCard, &WK_MifareCard, sizeof(t_MIF_CARDFORM) );
				PayData.teiki.id = 0;	// ��d�g�p�G���[���
				ret = 0;				// �J�[�hRead�f�[�^����Z�b�g
			}
		}else{
			if(ret == 1){					// CRC�G���[
				if(MifCard.type == 0x75){								// GT�t�H�[�}�b�g
					memset(uc_GtAlmPrm,0x00,10);						// 0�N���A
					memcpy(&uc_GtAlmPrm[0],&MifCard.pk_no,4);			// ���ԏ�No�Z�b�g
					memcpy(&uc_GtAlmPrm[4],&MifCard.pas_id,2);			// �����ID�Z�b�g
					alm_chk2( ALMMDL_SUB2, ALARM_GT_MIFARE_READ_CRC_ERR, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
					if(prm_get( COM_PRM, S_TIK, 25, 1, 1 ) == 1){
						GT_Mifare_RCRC_NG = 1;				// �u���b�N�R�������ݕs�ǎ��A���`�p�XOFF�Ƃ���B
						ret = 0;							// �J�[�hRead�f�[�^����Z�b�g
					}
				}
			}
			wk  = Mifare_WrtNgDataSearch( &MIF_CARD_SID[0] );
				// �����ݎ��s�e�[�u���ɓo�^����Ă��邩����
			if( 0xffff != wk ){	// ����
				// �O��Read�l��L���Ƃ���
				memcpy( &MifCard, &MIF_WriteNgCard_ReadData.data[wk].Data, sizeof(t_MIF_CARDFORM) );
				PayData.teiki.id = 0;	// ��d�g�p�G���[���
				ret = 0;				// �J�[�hRead�f�[�^����Z�b�g
			}
		}
		if( 0 == ret ){	// ��M�f�[�^����i�ϊ������j
			memcpy( &MIF_LastReadCardData.Sid,  &MIF_CARD_SID[0], 4);
			memcpy( &MIF_LastReadCardData.Data, &MifCard,
				sizeof(t_MIF_CARDFORM) );
																			// Ope�p �G���A�ɕۑ�
		}
		else{															// �G���[ (CRC-NG�Ȃǁj
			if(ret == 1){									// CRC�G���[
				if(MifCard.type == 0x75){
					return( 47 );							// GT�t�H�[�}�b�g
				}else{
					return( 2 );							// APS�t�H�[�}�b�g
				}
			}else if(ret == 2){								// ID�G���[
				return( 8 );
			}else{
				return( 2 );
			}
		}
	}

	ret = 0;
	for( ; ; ){
		if( (MifCard.type != 0x70) && (MifCard.type != 0x75) ){									// ��������Mifare���?
			ret = 13;												// ��ʋK��O
			break;
		}
		if(MifCard.type == 0x70){								// APS�t�H�[�}�b�g
			ul_AlmPrm = (ulong)MifCard.pk_no;
			ul_AlmPrm *= 100000L;								// ��������ԏ�set
			ul_AlmPrm += (ulong)MifCard.pas_id;					// �����IDset(1�`12000)
		}else{													// GT�t�H�[�}�b�g
			memset(uc_GtAlmPrm,0x00,10);						// 0�N���A
			memcpy(&uc_GtAlmPrm[0],&MifCard.pk_no,4);			// ���ԏ�No�Z�b�g
			memcpy(&uc_GtAlmPrm[4],&MifCard.pas_id,2);			// �����ID�Z�b�g
		}
		if( 0L == (ulong)MifCard.pk_no ){
			ret = 1;											// ���ԏꇂ�װ
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{												// GT�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}
		if(( prm_get( COM_PRM, S_SYS, 70, 1, 6 ) == 1 )&&				// ��{������g�p��
		   ( CPrmSS[S_SYS][1] == (ulong)MifCard.pk_no )){ 				// ��{���ԏꇂ?
			c_ptyp = KIHON_PKNO;										// ��{
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 1 ) == 1 )&&			// �g��1������g�p��
				( CPrmSS[S_SYS][2] == (ulong)MifCard.pk_no )){			// �g��1���ԏꇂ?
			c_ptyp = KAKUCHOU_1;										// �g��1
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 2 ) == 1 )&&			// �g��2������g�p��
				( CPrmSS[S_SYS][3] == (ulong)MifCard.pk_no )){			// �g��2���ԏꇂ?
			c_ptyp = KAKUCHOU_2;										// �g��2
		}
		else if(( prm_get( COM_PRM, S_SYS, 70, 1, 3 ) == 1 )&&			// �g��3������g�p��
				( CPrmSS[S_SYS][4] == (ulong)MifCard.pk_no )){			// �g��3���ԏꇂ?
			c_ptyp = KAKUCHOU_3;										// �g��3
		}
		else{
			ret = 1;													// ���ԏꇂ�װ
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{												// GT�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}

		ret = Ope_GT_Settei_Check_For_Mif( (MifCard.type == 0x75 ? 1:0), (ushort)c_ptyp );
		if( ret == 1 ){
			if( MifCard.type == 0x70 ){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{												// GT�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			ret = 33;
			break;
			
		}else if( ret == 2 ){
			ret = 1;											// ���ԏꇂ�װ
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{												// GT�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}			

		if( ((MifCard.type == 0x75) && ((GTF_PKNO_LOWER > MifCard.pk_no) || (MifCard.pk_no > GTF_PKNO_UPPER))) ||
			((MifCard.type == 0x70) && ((APSF_PKNO_LOWER > MifCard.pk_no) || (MifCard.pk_no > APSF_PKNO_UPPER))) ){//���ԏ�No�͈̓`�F�b�N

			ret = 1;												// ���ԏꇂ�װ
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_PARKING_NO_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{												// GT�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}

		s_ptbl = ReadPassTbl( MifCard.pk_no, MifCard.pas_id, us_psts );
		if( s_ptbl == -1 ){											// �װ(���l�ُ�)
			ret = 13;												// ��ʋK��O�װ
			break;
		}
		if( us_psts[0] ){											// �����o�^
			ret = 5;												// �����װ
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_MUKOU_PASS_USE, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}

		if( (short)prm_get( COM_PRM,S_PAS,(short)(5+10*(MifCard.pas_knd-1)),1,1 ) == 1 ){	// �����؂ꎞ��t����ݒ�
			c_prm = (char)prm_get( COM_PRM,S_PAS,(short)(5+10*(MifCard.pas_knd-1)),1,2 );	// �������ݒ�Get
		}else{
			c_prm = 0;
		}

		if( c_prm == 1 || c_prm == 2 ){								// ������ or �J�n��������
			us_sday = dnrmlzm( 1980, 3, 1 );						// 1980�N3��1��
		}else{
			if( chkdate( (short)MifCard.limit_s.Year,				// �L���J�n������NG?
			             (short)MifCard.limit_s.Mon,
			             (short)MifCard.limit_s.Day ) ){
				return( 2 );										// �ް��ُ�
			}
			us_sday = dnrmlzm( (short)MifCard.limit_s.Year, (short)MifCard.limit_s.Mon, (short)MifCard.limit_s.Day );	// �L���J�n�����
		}

		CRD_DAT.PAS.std_end[0] = (char)(MifCard.limit_s.Year % 100);
		CRD_DAT.PAS.std_end[1] = MifCard.limit_s.Mon;
		CRD_DAT.PAS.std_end[2] = MifCard.limit_s.Day;

		if( c_prm == 1 || c_prm == 3 ){								// ������ or �I����������
			us_eday = dnrmlzm( 2079, 12, 31 );						// 2079�N12��31��
		}else{
			if( chkdate( (short)MifCard.limit_e.Year,				// �L���I��������NG?
			             (short)MifCard.limit_e.Mon,
			             (short)MifCard.limit_e.Day ) ){
				return( 2 );										// �ް��ُ�
			}
			us_eday = dnrmlzm( (short)MifCard.limit_e.Year, (short)MifCard.limit_e.Mon, (short)MifCard.limit_e.Day );	// �L���I�������
		}

		CRD_DAT.PAS.std_end[3] = (char)(MifCard.limit_e.Year % 100);
		CRD_DAT.PAS.std_end[4] = MifCard.limit_e.Mon;
		CRD_DAT.PAS.std_end[5] = MifCard.limit_e.Day;

		if( !rangechk( 1, 15, MifCard.pas_knd ) ||					// �����ʔ͈͊O
			!rangechk( 0, 3, ( MifCard.io_stat & 0x0F ) ) )					// �ð���͈͊O
		{
			ret = 13;												// ��ʋK��O�װ
			break;
		}

		if( ryo_buf.syubet < 6 ){
			// ���Z�Ώۂ̗������A�`F(0�`5)
			data_adr = 10*(MifCard.pas_knd-1)+9;					// �g�p�\������ʂ��ް����ڽ�擾
			data_pos = (char)(6-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
		}
		else{
			// ���Z�Ώۂ̗������G�`L(6�`11)
			data_adr = 10*(MifCard.pas_knd-1)+10;					// �g�p�\������ʂ��ް����ڽ�擾
			data_pos = (char)(12-ryo_buf.syubet);					// �g�p�\������ʂ��ް��ʒu�擾
		}
		if( prm_get( COM_PRM, S_PAS, data_adr, 1, data_pos ) ){		// �g�p�s�ݒ�H
			ret = 25;												// ���̎Ԏ�̌�
			break;
		}

		if( PayData.teiki.id != 0 ){								// ��d�g�p?
			ret = 9;												// ��d�g�p�װ
			break;
		}

		if( CPrmSS[S_TIK][9] ){										// n�������L��?
			if( (PassIdBackupTim) &&								// n�������N����?
				(PassPkNoBackup == MifCard.pk_no) &&
				(PassIdBackup   == MifCard.pas_id) ){				// �O��Ɠ��궰��
				ret = 9;											// ��d�g�p�װ(n�������װ)
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_N_MINUTE_RULE, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_N_MINUTE_RULE, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
				break;
			}
		}

		if(( (MifCard.io_stat & 0x0F) == 0 )&&								// �ް���?
		   ( CPrmSS[S_TIK][8] == 0 )){								// �������ǂ܂Ȃ��ݒ�?
			ret = 26;												// �������g�p�s�´װ
			break;
		}

		if( us_sday > us_eday ){									// �L�������ް��ُ�(�J�n���I��)
			ret = 13;												// ��ʋK��O�װ
			break;
		}

		/** ����L�������`�F�b�N����(�`�F�b�N������NT-7700�ɍ��킹��) **/
		KigenCheckResult = Ope_PasKigenCheck( us_sday, us_eday, (short)MifCard.pas_knd, CLK_REC.ndat, CLK_REC.nmin );
											// ����L�������`�F�b�N�i�߂�F0=�����J�n�����O�C1=�L���������C2=�����I��������j

		if( (1 != KigenCheckResult) &&								// �����؂�
			(prm_get( COM_PRM,S_PAS,(short)(5+10*(MifCard.pas_knd-1)),1,1 ) == 0 ) ){	// �����؂ꎞ��t���Ȃ��ݒ�?

			/*** �����؂�Ŋ����؂�����t���Ȃ��ݒ�̎� ***/
			if( 0 == KigenCheckResult ){							// �����O
				ret = 6;											// �����O�װ
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
				break;
			}
			else{													// �����؂�
				ret = 3;											// �����؂�װ
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_VALIDITY_TERM_OUT, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
				break;
			}
			break;
		}

		w_syasyu = 0;
		c_prm = (char)CPrmSS[S_PAS][1+10*(MifCard.pas_knd-1)];		// �g�p�ړI�ݒ�Get
		if( !rangechk( 1, 14, c_prm ) ){							// ����g�p�ړI�ݒ�͈͊O
			if (c_prm == 0) {										// �g�p�ړI�F���g�p
				ret = 26;											// �ݒ�װ
			}
			else {
				ret = 13;											// ��ʋK��O
			}
			if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
				alm_chk2( ALMMDL_SUB2, ALARM_NOT_USE_TICKET, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
			}else{
				alm_chk2( ALMMDL_SUB2, ALARM_GT_NOT_USE_TICKET, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
			}
			break;
		}
		if( rangechk( 3, 14, c_prm ) ){								// ����Ԏ�؊�?
			w_syasyu = c_prm - 2;									// �Ԏ�؊��p�Ԏ��
			if ( prm_get( COM_PRM,S_SHA,(short)(1+6*(w_syasyu-1)),2,5 ) == 0L ) {	// �Ԏ�ݒ�Ȃ�
				ret = 27;											// ������ʴװ
				break;
			}
			if(( ryo_buf.waribik )||								// �����ς�?
			   ( ryo_buf.zankin == 0 ))								// �c��0�~�̎�
			{
				ret = 30;											// ���Z���Դװ
				break;
			}
		}
		// ���z���Z��̒�����͖���
		i = is_paid_remote(&PayData);
		if (i >= 0 &&
		   ( PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU || 
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
//// MH322914(S) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
//		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) || 
//// MH322914(E) K.Onodera 2016/09/15 AI-V�Ή��F�U�֐��Z
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
		   ( PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE && !vl_frs.antipassoff_req)) ) {
			ret = 8;
			break;
		}
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
		i = is_ParkingWebFurikae( &PayData );
		if (i >= 0 &&  ( PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2 && !vl_frs.antipassoff_req) ){
			ret = 8;
			break;
		}
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
		if(	(OPECTL.Pay_mod == 2)&&						// �C�����Z
			(vl_frs.antipassoff_req)&&					// �C����������g�p
			(prm_get(COM_PRM, S_TYP, 98, 1, 4)==1)){	// ��������������
			if( syusei[vl_frs.lockno-1].tei_id != CRD_DAT.PAS.cod ){
				ret = 13;											// ��ʋK��O
				break;
			}
		}
		ryo_buf.pass_zero = 0;										// ���������0�~���Z�v���Ȃ�
		if( chk_for_inquiry(1) ) {									// �⍇����?
			if( !ERR_CHK[mod_ntibk][1] ){							// NTNET IBK �ʐM����?
				OPECTL.ChkPassSyu = 2;								// ������⍇��������2=Mifare�����
				OPECTL.ChkPassPkno = (ulong)MifCard.pk_no;			// ������⍇�������ԏꇂ
				OPECTL.ChkPassID = MifCard.pas_id;					// ������⍇���������ID
				// �����ł͑��M�v���������Ȃ��Bopemain��ELE_EVT_STOP�œd�q�}�̂̒�~��҂��Ă��瑗�M����
				ret = 99;
				break;
			}else{
				OPECTL.ChkPassSyu = 0xfe;							// �ʐM�s��
				wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );		// �ʐM�s�ǎ��̐ݒ�
				if( wk == 0 ){										// NG�Ŏg�p�s��
					ret = 29;										// �ʐM�s��or������ѱ��
					break;
				}else if( wk == 1 ){								// ����0�~
					ryo_buf.pass_zero = 1;							// ���������0�~���Z�v������
				}
			}
		}

		if( PassExitTimeTblRead( MifCard.pk_no, MifCard.pas_id, &extim ) ){
			// �O�񏑍��ݎ��s�����̂ŏo�Ɏ�����O�񏑍��ނ͂������������ɂ���B
			memcpy( &MifCard.ext_tm, &extim, sizeof( date_time_rec ));
		}

		if( OPECTL.ChkPassSyu == 2 ){								// ������⍇����̖⍇�����ʎ�M
			memcpy( &MifCard.ext_tm, &PassChk.OutTime, sizeof( date_time_rec ) );	// ����������������M�ް��ɍX�V
		}
		else if( OPECTL.ChkPassSyu == 0xff ){						// ������⍇����̖⍇�����ʎ�M��ѱ��
			wk = (short)prm_get( COM_PRM,S_NTN,36,1,1 );			// �⍇��������ѱ�Ď��̐ݒ�
			if( wk == 0 ){											// NG�Ŏg�p�s��
				ret = 29;											// �ʐM�s��or������ѱ��
				break;
			}else if( wk == 1 ){									// ����0�~
				ryo_buf.pass_zero = 1;								// ���������0�~���Z�v������
			}
		}
		if(( DO_APASS_CHK )&&
		   ( prm_get( COM_PRM,S_PAS,(short)(2+10*(MifCard.pas_knd-1)),1,1 ) )&&	// ���o��������ݒ�?
		   ( OPECTL.ChkPassSyu != 0xff )&&							// ������⍇����̖⍇�����ʎ�M��ѱ�ĈȊO
		   ( GT_Mifare_RCRC_NG == 0 ) &&							// Mifare�ǎ��CRC�G���[���������o�`�F�b�N����?
		   ( OPECTL.ChkPassSyu != 0xfe )){							// �ʐM�s��
			if((MifCard.io_stat & 0x0f) != 0 ){
				wk = CLK_REC.year;
				us_sday = dnrmlzm( wk, (short)MifCard.ext_tm.Mon, (short)MifCard.ext_tm.Day );	// ���ތ���ɰ�ײ��

				if( us_sday > CLK_REC.ndat ){
					wk--;											// �NϲŽ
					us_sday = dnrmlzm( wk, (short)MifCard.ext_tm.Mon, (short)MifCard.ext_tm.Day );	// ���ތ���ɰ�ײ��
				}
				wk = dnrmlzm( car_in_f.year, (short)car_in_f.mon, (short)car_in_f.day );

				if( us_sday > wk ){									// ���ތ��� > ���Ɍ���?
					ret = 4;										// ���o�װ
					break;
				}
				if( us_sday == wk ){
					wk = tnrmlz( 0, 0, (short)car_in_f.hour, (short)car_in_f.min );

					us_sday = tnrmlz( 0, 0, (short)MifCard.ext_tm.Hour, (short)MifCard.ext_tm.Min );
					if( us_sday > wk ){
						ret = 4;									// ���o�װ
						break;
					}
				}
			}
		}
		if( KaisuuWaribikiGoukei || c_pay || PayData_Sub.pay_ryo ){ // ����߁E�񐔌��g�p���͒�����̎g�p��s�Ƃ���
			ret = 9;												// ��d�g�p�װ
		}
		break;
	}

	if( ret == 4 ){													// ���o�װ?
		for( wk=0; wk<TKI_CYUSI_MAX; wk++ ){
			if( MifCard.pas_id == tki_cyusi.dt[wk].no &&			// ���~�ް��ƈ�v?
				(ulong)MifCard.pk_no == tki_cyusi.dt[wk].pk ){
				tkcyu_ichi = (char)(wk + 1);						// ���~�ǎ�ʒu
				ret = 0;											// ����OK
				break;
			}
		}
		if( ret == 4 ){
			if( chk_for_inquiry(2) ) {								// ����߽NG�̎��̂ݖ⍇����?
				if( !ERR_CHK[mod_ntibk][1] ){						// NTNET IBK �ʐM����?
					OPECTL.ChkPassSyu = 2;							// ������⍇��������2=Mifare�����
					OPECTL.ChkPassPkno = (ulong)MifCard.pk_no;		// ������⍇�������ԏꇂ
					OPECTL.ChkPassID = MifCard.pas_id;				// ������⍇���������ID
					// �����ł͑��M�v���������Ȃ��Bopemain��ELE_EVT_STOP�œd�q�}�̂̒�~��҂��Ă��瑗�M����
					ret = 99;
				}else{
					wk = (short)prm_get( COM_PRM,S_NTN,36,1,2 );	// �ʐM�s�ǎ��̐ݒ�
					if( wk == 0 ){									// NG�Ŏg�p�s��
						ret = 29;									// �ʐM�s��or������ѱ��
					}else if( wk == 1 ){							// ����0�~
						ryo_buf.pass_zero = 1;						// ���������0�~���Z�v������
						ret = 0;
					}else if( wk == 2 ){							// ���ގ����Ő��Z
						ret = 0;
					}
				}
			}
		}
	}

	if( ret == 4){		//���o�װ?
		if(MifCard.type == 0x70){							// APS�t�H�[�}�b�g
			alm_chk2( ALMMDL_SUB2, ALARM_ANTI_PASS_NG, 2, 2, 1, (void *)&ul_AlmPrm );	// ��ݾڰ��ݱװѓo�^
		}else{
			alm_chk2( ALMMDL_SUB2, ALARM_GT_ANTI_PASS_NG, 2, 1, 1, (void *)&uc_GtAlmPrm );	// ��ݾڰ��ݱװѓo�^
		}
	}

	if( ret == 99 && MifStat == MIF_WROTE_FAIL ){								// �����ݎ��s��̍����
		ret = 0;											// ����I���Ƃ���
	}

	if( ret != 99 ){												// �⍇�����Ă��Ȃ�?
		OPECTL.ChkPassSyu = 0;										// ������⍇��������ر
		OPECTL.ChkPassPkno = 0L;									// ������⍇�������ԏꇂ�ر
		OPECTL.ChkPassID = 0L;										// ������⍇���������ID�ر
		blink_end();												// �_�ŏI��
	}

	if( ret == 0 ){													// ����OK?
		if( w_syasyu ){
			vl_now = V_SYU;											// ��ʐ؊�
			syashu = (char)w_syasyu;								// �Ԏ�
		}else{														// �Ԏ�؊��łȂ�
			vl_now = V_TSC;											// �����(���Ԍ����p�L��)
		}
		// �}������ް������ʴر�ɾ��� �ivl_paschg()�֐���ق��邽�߁j
		CRD_DAT.PAS.knd = MifCard.pas_knd;				// ��������
		CRD_DAT.PAS.sts = MifCard.io_stat & 0x0f;		// ������ð��(�ǎ掞)
		CRD_DAT.PAS.cod = MifCard.pas_id;				// �����id
		memcpy( CRD_DAT.PAS.trz, &MifCard.ext_tm.Mon, 4 );		// ������������
		CRD_DAT.PAS.pno = MifCard.pk_no;				// ���ԏ�ԍ�

		// Mifares��p�֐��Ƃ��Ďg�p���Ă���vl_mifpaschg�͎g�p���Ȃ��悤�ɂ��Avl_paschg�����ʊ֐��Ƃ��Ďg�p����
		// �p�ɏC������
		vl_paschg();

		// �}������ް����ꎞ�ۑ��ر�ɾ��ށiOK�m��� PatData�֓o�^�j
		InTeiki_PayData_Tmp.syu 		 = (uchar)CRD_DAT.PAS.knd;	// ��������
		InTeiki_PayData_Tmp.status 		 = (uchar)CRD_DAT.PAS.sts;	// ������ð��(�ǎ掞)
		InTeiki_PayData_Tmp.id 			 = CRD_DAT.PAS.cod;			// �����id
		InTeiki_PayData_Tmp.pkno_syu 		 = c_ptyp;			// ��������ԏ�m���D��� (0-3:��{,�g��1-3)
		InTeiki_PayData_Tmp.update_mon 	 	 = 0;						// �X�V����
		InTeiki_PayData_Tmp.s_year 		 = MifCard.limit_s.Year;					// �L�������i�J�n�F�N�j
		InTeiki_PayData_Tmp.s_mon 		 = MifCard.limit_s.Mon;		// �L�������i�J�n�F���j
		InTeiki_PayData_Tmp.s_day 		 = MifCard.limit_s.Day;		// �L�������i�J�n�F���j
		InTeiki_PayData_Tmp.e_year 		 = MifCard.limit_e.Year;					// �L�������i�I���F�N�j
		InTeiki_PayData_Tmp.e_mon 		 = MifCard.limit_e.Mon;		// �L�������i�I���F���j
		InTeiki_PayData_Tmp.e_day 		 = MifCard.limit_e.Day;		// �L�������i�I���F���j
		memcpy( InTeiki_PayData_Tmp.t_tim, CRD_DAT.PAS.trz, 4 );	// ������������
		InTeiki_PayData_Tmp.update_rslt1 = 0;						// ����X�V���Z���̍X�V����			�i�@OK�F�X�V�����@�^�@NG�F�X�V���s�@�j
		InTeiki_PayData_Tmp.update_rslt2 = 0;						// ����X�V���Z���̃��x�����s����	�i�@OK�F���픭�s�@�^�@NG�F���s�s�ǁ@�j
	}
	return( ret );
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare�����ݏ���                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MifareDataWrt( void )                                   |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	MifareDataWrt( void )
{

	MifCard.io_stat &= 0xF0;											// ���ޓ��o�ɽð���X�V(�N���A)
	MifCard.io_stat |= 0x01;											// ���ޓ��o�ɽð���X�V(�o��)
	memcpy( &MifCard.ext_tm.Year, &CLK_REC, sizeof( date_time_rec ));	// �o�Ɏ����X�V

	OpMif_snd( 0xA4, 0 );											// Mifare������
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare�����ݏ���(for ������s���)                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : MifareDataWrt2( void )                                  |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	MifareDataWrt2( void )
{
	OpMif_snd2( 0xA4, 0 );											// Mifare������
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare�����ݎ��s���̃f�[�^�X�V                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : Request	�v������									   |*/
/*|							1=�S�ر�A2=1���폜�A3=1���o�^				   |*/
/*|				   pData    Request���ɓ��e���قȂ�						   |*/
/*|							Request=1 �� �Ӗ��Ȃ��i0L��OK�j				   |*/
/*|							Request=2 �� �폜����SID���w��				   |*/
/*|										 ���̓��e�͎w��s�v				   |*/
/*|							Request=3 �� �o�^������e�i�S���ڎw��K�v�j	   |*/
/*|										 ����SID���ޏ�񂪂���ꍇ��	   |*/
/*|										 ���ް����폜���ĐV�K�o�^���Ȃ���  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	�E�{�֐��ł�ܰ��ر�ɏ���Ă���݂̂ŁA���ް��̍X�V�͎��̊֐��ōs���B|*/
/*|	�E��d�Ɋւ��铮��́A�����ް����󂳂Ȃ��悤�ɂ��邱�Ƃ���Ƃ���B	   |*/
/*|	  ܰ��ر�o�^�O�ɓd�����������ꍇ�͕ۏ؂��Ȃ��B						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-11-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Mifare_WrtNgDataUpdate( uchar Request, void *pVoid )
{
	t_MIF_CardData *pData = (t_MIF_CardData*)pVoid;

	ushort	Index;

	switch( Request ){
	case	1:														// �S�ر
		/* ܰ��ر���ް���� */
		MIF_WriteNgCard_ReadData.wk_count = 0;
		memset( &MIF_WriteNgCard_ReadData.wk_data[0], 0, sizeof(t_MIF_CardData)*MIF_WRITE_NG_BUF_COUNT );
		/* ���ް��X�V(��d�ۏ�) */
		MIF_WriteNgCard_ReadData.f_Phase = 0x55;					// ܰ��ر���ް���Ċ���
		Mifare_WrtNgDataUpdate_Exec();
		break;

	case	2:														// 1���폜�w��
		/* pData->Sid[] �ɑΏ��ް����� */
		Index = Mifare_WrtNgDataSearch( &(pData->Sid[0]) );			// ����SID����
		if( 0xffff == Index )										// ����SID�Ȃ�
			return;													// �I��

		/* ���������ް��̑O�܂ł�ܰ��ޯ̧��copy */
		if( 0 != Index ){											// �擪�ް��ł͂Ȃ�
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
					&MIF_WriteNgCard_ReadData.data[0], sizeof(t_MIF_CardData)*(Index) );
		}
		/* ���������ް��𔲂����� �Ō�܂ł�ܰ��ޯ̧��copy */
		if( (MIF_WRITE_NG_BUF_COUNT - 1) != Index ){				// �Ō���ް��ł͂Ȃ�
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[Index], 
					&MIF_WriteNgCard_ReadData.data[Index+1], 
						sizeof(t_MIF_CardData)*((MIF_WRITE_NG_BUF_COUNT - 1) - Index) );
		}

		/* ���܂�Full�����������疖����0�ر���� */
		if( MIF_WRITE_NG_BUF_COUNT <= MIF_WriteNgCard_ReadData.count ){	// Buffer full
			memset( &MIF_WriteNgCard_ReadData.wk_data[MIF_WRITE_NG_BUF_COUNT - 1], 0, sizeof(t_MIF_CardData) );
		}

		/* �o�^���� - 1 */
		MIF_WriteNgCard_ReadData.wk_count = MIF_WriteNgCard_ReadData.count - 1;

		/* ���ް��X�V(��d�ۏ�) */
		MIF_WriteNgCard_ReadData.f_Phase = 0x55;					// ܰ��ر���ް���Ċ���
		Mifare_WrtNgDataUpdate_Exec();
		break;

	case	3:														// 1���o�^
		/* ���߂ɓ���SID�ް�������΍폜���� */
		Index = Mifare_WrtNgDataSearch( &(pData->Sid[0]) );			// ����SID����
		if( 0xffff != Index ){										// ����SID����
			/* ���������ް��̑O�܂ł�ܰ��ޯ̧��copy */
			if( 0 != Index ){											// �擪�ް��ł͂Ȃ�
				memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
						&MIF_WriteNgCard_ReadData.data[0], sizeof(t_MIF_CardData)*(Index) );
			}
			/* ���������ް��𔲂����� �Ō�܂ł�ܰ��ޯ̧��copy */
			if( (MIF_WRITE_NG_BUF_COUNT - 1) != Index ){				// �Ō���ް��ł͂Ȃ�
				memcpy( &MIF_WriteNgCard_ReadData.wk_data[Index], 
						&MIF_WriteNgCard_ReadData.data[Index+1], 
							sizeof(t_MIF_CardData)*((MIF_WRITE_NG_BUF_COUNT - 1) - Index) );
			}
			/* ���܂�Full�����������疖����0�ر���� */
			if( MIF_WRITE_NG_BUF_COUNT <= MIF_WriteNgCard_ReadData.count ){	// Buffer full
				memset( &MIF_WriteNgCard_ReadData.wk_data[MIF_WRITE_NG_BUF_COUNT - 1], 0, sizeof(t_MIF_CardData) );
			}

			/* �o�^���� - 1 */
			MIF_WriteNgCard_ReadData.wk_count = MIF_WriteNgCard_ReadData.count - 1;
		}
		else{	/* ������� �ȍ~ܰ��ر�ō�Ƃ���̂� �ް���copy���� */
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
					&MIF_WriteNgCard_ReadData.data[0], sizeof(t_MIF_CardData)*MIF_WRITE_NG_BUF_COUNT );

			MIF_WriteNgCard_ReadData.wk_count = MIF_WriteNgCard_ReadData.count;
		}

		/* 1���o�^���� */
		if( MIF_WRITE_NG_BUF_COUNT <= MIF_WriteNgCard_ReadData.wk_count ){	// Buffer full
			/* �擪�P������ */
			memcpy( &MIF_WriteNgCard_ReadData.wk_data[0], 
					&MIF_WriteNgCard_ReadData.wk_data[1], sizeof(t_MIF_CardData)*(MIF_WRITE_NG_BUF_COUNT - 1) );
			MIF_WriteNgCard_ReadData.wk_count = (MIF_WRITE_NG_BUF_COUNT - 1);	// �o�^����-1
		}

		/* ܰ��ر�̖����ɓo�^ */
		memcpy( &MIF_WriteNgCard_ReadData.wk_data[MIF_WriteNgCard_ReadData.wk_count], pData, sizeof(t_MIF_CardData) );
		++MIF_WriteNgCard_ReadData.wk_count;						// �o�^����+1

		/* ���ް��X�V(��d�ۏ�) */
		MIF_WriteNgCard_ReadData.f_Phase = 0x55;					// ܰ��ر���ް���Ċ���
		Mifare_WrtNgDataUpdate_Exec();
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare�����ݎ��s���̃f�[�^�X�V                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : none													   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*|	t_MIF_WriteNgCard_ReadData ��ܰ��ر���e�����ް��ر��copy����		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-11-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Mifare_WrtNgDataUpdate_Exec( void )
{
	if( 0x55 != MIF_WriteNgCard_ReadData.f_Phase )					// �ް���Ċ����ł͂Ȃ�(copy�s�v)
		return;

	MIF_WriteNgCard_ReadData.count = MIF_WriteNgCard_ReadData.wk_count;
	memcpy( &MIF_WriteNgCard_ReadData.data[0], &MIF_WriteNgCard_ReadData.wk_data[0],
										 sizeof(t_MIF_CardData)*MIF_WRITE_NG_BUF_COUNT );
	MIF_WriteNgCard_ReadData.f_Phase = 0;							// copy����
}

/*[]----------------------------------------------------------------------[]*/
/*| Mifare�����ݎ��s�ر���瓯��SID�������ް�����                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : pSid[4]  ��������SID									   |*/
/*| RETURN VALUE : 0xffff = �Ώۃf�[�^�Ȃ�                                 |*/
/*|				   ��L�ȊO�i0�`�j= �Ώ��ް���Index						   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : okuda                                                   |*/
/*| Date         : 2005-11-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	Mifare_WrtNgDataSearch( uchar pSid[] )
{
	ushort	i, retus;

	retus = 0xffff;

	/* fail safe */
	if( MIF_WRITE_NG_BUF_COUNT < MIF_WriteNgCard_ReadData.count)
		MIF_WriteNgCard_ReadData.count = 0;

	for( i=0; i<MIF_WriteNgCard_ReadData.count; ++i ){				// �o�^������
		if( 0 == memcmp( pSid, MIF_WriteNgCard_ReadData.data[i].Sid, 4 ) ){	// ����SID����
			retus = i;
			break;
		}
	}

	return	retus;
}

/*[]-----------------------------------------------------------------------[]*/
/*| GT̫�ϯĐݒ�Mifare��������												|*/
/*[]-----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Ope_GT_Settei_Check_For_Mif()                            |*/
/*| PARAMETER    : gt_flg = �ǂݎ��������GTorAPS̫�ϯ�						|*/
/*|				   pno_syu= ��{,�g��1�`3�̔ԍ�								|*/
/*| RETURN VALUE : 0=�ݒ�OK,1=�ݒ�NG									    |*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : namioka			                                        |*/
/*| Date         : 2008/06/16                                               |*/
/*| UpDate       :                                                          |*/
/*[]-------------------------------------- Copyright(C) 2008 AMANO Corp.---[]*/
static short Ope_GT_Settei_Check_For_Mif( uchar gt_flg, ushort pno_syu )
{
short	ret;

	ret = 0;
	if(prm_get( COM_PRM,S_SYS,11,1,2) == 1){									// GT�ݒ�
		if( gt_flg == 1 ){														// ذ���ް���GT̫�ϯ�
			if( !prm_get( COM_PRM,S_SYS,12,1,mgtype[pno_syu] )){				// �ݒ�װ�Ƃ���
				ret = 1;
			}
		}//else�̏ꍇ�́A�f�[�^������ɓǂݏo���Ă��Ȃ��ׁA�w�g�p�s���ł��x�Ƃ����G���[�ɂȂ�
	}else{																		// APS/GT�ݒ�
		if( gt_flg == 1 ){														// ذ���ް���GT̫�ϯ�
			if( !prm_get( COM_PRM,S_SYS,12,1,mgtype[pno_syu] )){
				ret = 2;														// ���̒��ԏ�̌�(���ԏ�No�װ)�Ƃ���
			}
		}else{																	// ذ���ް���APS̫�ϯ�
			if( prm_get( COM_PRM,S_SYS,12,1,mgtype[pno_syu] )){
				ret = 2;														// ���̒��ԏ�̌�(���ԏ�No�װ)�Ƃ���
			}
		}
	}
	return( ret );
}
