/*[]----------------------------------------------------------------------[]*/
/*| �ޑK�Ǘ��֘A                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 2002.02.01                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"pri_def.h"
#include	"mnt_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"raudef.h"
#include	"ntnet_def.h"
#include	"oiban.h"

/*[]----------------------------------------------------------------------[]*/
/*| �ޑK�Ǘ��̌��ݕۗL���������߂�                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_gen                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*| Update		 :                                                         |*/
/*| Update		 :                                                         |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_gen( void )
{
	int	i;
	long cl;

	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	ac_flg.turi_syu = 2;

	for( i = 0; i < 4; i++ ){
		cl = (long)turi_kan.turi_dat[i].zen_mai;
		cl += turi_kan.turi_dat[i].sei_nyu;
		cl += turi_kan.turi_dat[i].hojyu;
		cl -= turi_kan.turi_dat[i].sei_syu;
		if( cl < 0L ){
			cl = 0L;
		}
		if( turi_kan.turi_dat[i].kyosei != 0 ){ /* ������؂���? */
			if( cl >= (long)turi_kan.turi_dat[i].kyosei ){
				cl -= (long)turi_kan.turi_dat[i].kyosei;
			}else{
				/* ������ؖ����͑S�������o���Ɩ����𐳊m�ɔc���ł��Ȃ��ׁA�������� */
				turi_kan.turi_dat[i].kyosei = cl;
				cl = 0L;
			}
		}
		turi_kan.turi_dat[i].gen_mai = (unsigned short)cl;

		cl = (long)turi_kan.turi_dat[i].yzen_mai;
		cl -= turi_kan.turi_dat[i].ysei_syu;
		if( cl < 0L ){
			cl = 0L;
		}
		if( turi_kan.turi_dat[i].ykyosei != 0 ){ /* ������؂���? */
			if( cl >= (long)turi_kan.turi_dat[i].ykyosei ){
				cl -= (long)turi_kan.turi_dat[i].ykyosei;
			}else{
				/* ������ؖ����͑S�������o���Ɩ����𐳊m�ɔc���ł��Ȃ��ׁA�������� */
				turi_kan.turi_dat[i].ykyosei = cl;
				cl = 0L;
			}
		}
		turi_kan.turi_dat[i].ygen_mai = (unsigned short)cl;
	}

	ac_flg.turi_syu = 0;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Z�������ɓ��o�������i�z�̂݁j���Ă���                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_pay                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_pay( void )
{
	int	i, k;

	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	ac_flg.turi_syu = 22;

	for( i = 0; i < 4; i++ ){
		k = ryo_buf.in_coin[i] - turi_dat.pay_safe[i];
		if( k > 0 ){
			turi_kan.turi_dat[i].sei_nyu += (long)k; 	/* ���Z���������� */
			turi_kan.turi_dat[i].gen_mai += k; 			/*���ۗL���� */
		}
	}
	for( i = 0; i < 4; i++ ){
		k = ryo_buf.out_coin[i];
		if( k > 0 ){
			turi_kan.turi_dat[i].sei_syu += (long)k;
			if( turi_kan.turi_dat[i].gen_mai >= k ){
				turi_kan.turi_dat[i].gen_mai -= k; 		/*���ۗL���� */
			}
		}
	}
	k = ryo_buf.out_coin[4]; /* 10yen�\�~ */
	if( k > 0 ){
		turi_kan.turi_dat[0].ysei_syu += (long)k;
		turi_kan.turi_dat[0].ygen_mai  -= (ushort)k;			// �\������ނ��略��������10�~�̖����������Z����
	}
	k = ryo_buf.out_coin[5]; /* 50yen�\�~ */
	if( k > 0 ){
		turi_kan.turi_dat[1].ysei_syu += (long)k;
		turi_kan.turi_dat[1].ygen_mai  -= (ushort)k;			// �\������ނ��略��������50�~�̖����������Z����
	}
	k = ryo_buf.out_coin[6]; /* 100yen�\�~ */
	if( k > 0 ){
		turi_kan.turi_dat[2].ysei_syu += (long)k;
		turi_kan.turi_dat[2].ygen_mai  -= (ushort)k;			// �\������ނ��略��������100�~�̖����������Z����
	}

// MH810104(S) R.Endo 2021/10/25 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6127 �y����w�E�z���Z�����シ���Ƀg�b�v�֖߂�ƁA���ݖ������X�V���ꂸ�A���K�Ǘ��f�[�^�����M����Ȃ��B
	if ( Make_Log_MnyMng(OpeNtnetAddedInfo.PayClass) ) {		// ���K�Ǘ����O�f�[�^�쐬
		Log_regist(LOG_MONEYMANAGE_NT);							// ���K�Ǘ����O�o�^
	}
// MH810104(E) R.Endo 2021/10/25 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6127 �y����w�E�z���Z�����シ���Ƀg�b�v�֖߂�ƁA���ݖ������X�V���ꂸ�A���K�Ǘ��f�[�^�����M����Ȃ��B

	ac_flg.turi_syu = 23;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���Ē����̏�����                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turian_ini                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_ini( void )
{
	int	i;
	short nu;

	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	ac_flg.turi_syu = 1;

	if( Tubu_cnt_set != 0 ){ /* �����ݒ肠��? */
		nu = (unsigned short)CPrmSS[S_KAN][3];		// 10�~�f�t�H���g����
		if( nu > TUB_MAX10 ) nu = TUB_MAX10;
		turi_kan.turi_dat[0].sin_mai = nu;

		nu = (unsigned short)CPrmSS[S_KAN][9];		// 50�~�f�t�H���g����
		if( nu > TUB_MAX50 ) nu = TUB_MAX50;
		turi_kan.turi_dat[1].sin_mai = nu;

		nu = (unsigned short)CPrmSS[S_KAN][15];		// 100�~�f�t�H���g����
		if( nu > TUB_MAX100 ) nu = TUB_MAX100;
		turi_kan.turi_dat[2].sin_mai = nu;

		nu = (unsigned short)CPrmSS[S_KAN][21];		// 500�~�f�t�H���g����
		if( nu > TUB_MAX500 ) nu = TUB_MAX500;
		turi_kan.turi_dat[3].sin_mai = nu;

		turi_kan.turi_dat[0].ysin_mai = (unsigned short)prm_get( COM_PRM,S_KAN,27,3,1 );	// 10�~�\�~�f�t�H���g����
		turi_kan.turi_dat[1].ysin_mai = (unsigned short)prm_get( COM_PRM,S_KAN,30,3,1 );	// 50�~�\�~�f�t�H���g����
		turi_kan.turi_dat[2].ysin_mai = (unsigned short)prm_get( COM_PRM,S_KAN,33,3,1 );	// 100�~�\�~�f�t�H���g����

		for( i = 0; i < 4; i++ ){
			turi_kan.turi_dat[i].gen_mai = turi_kan.turi_dat[i].sin_mai;
			turi_kan.turi_dat[i].ygen_mai = turi_kan.turi_dat[i].ysin_mai;
		}
	}

	ac_flg.turi_syu = 0;

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �ޑK�Ǘ�����̊e����                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Cash_proc( ope_kind )                                   |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hashimoto(AMANO)                                        |*/
/*| Date         : 2001-11-02                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	turikan_proc( short ope_kind )
{
	int	i;
	short kin = 0;
	short ckn[4];


	switch( ope_kind )
	{
		case MNY_CHG_BEFORE: // �ޑK�����ύX�̑O����
			turi_dat.turi_in = 0x01ff;
			turikan_gen();
			turi_dat.coin_dsp[0] = turi_kan.turi_dat[0].gen_mai;
			turi_dat.coin_dsp[1] = turi_kan.turi_dat[1].gen_mai;
			turi_dat.coin_dsp[2] = turi_kan.turi_dat[2].gen_mai;
			turi_dat.coin_dsp[3] = turi_kan.turi_dat[3].gen_mai;
			turi_dat.coin_dsp[4] = turi_kan.turi_dat[0].ygen_mai; // 10�~�\�~
			turi_dat.coin_dsp[5] = turi_kan.turi_dat[1].ygen_mai; // 50�~�\�~
			turi_dat.coin_dsp[6] = turi_kan.turi_dat[2].ygen_mai; // 100�~�\�~
			cn_stat( 24, 0 ); // CREM OFF, ������؋֎~
			// �ޑK�Ǘ����v����Ď��ɎQ�Ƃ���׸ر���Ă���
			memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); 	// �o�າݶ���(�ۗL�����ω���) 
			break;
		case MNY_CHG_10YEN: // 10�~�����ύX
			turi_dat.turi_in &= (~0x0010);
			break;
		case MNY_CHG_50YEN: // 50�~�����ύX
			turi_dat.turi_in &= (~0x0020);
			break;
		case MNY_CHG_100YEN: // 100�~�����ύX
			turi_dat.turi_in &= (~0x0040);
			break;
		case MNY_CHG_500YEN: // 500�~�����ύX
			turi_dat.turi_in &= (~0x0080);
			break;
		case MNY_CHG_10SUBYEN: // 10�~SUB�����ύX
			turi_dat.turi_in &= (~0x0004);
			break;
		case MNY_CHG_50SUBYEN: // 50�~SUB�����ύX
			turi_dat.turi_in &= (~0x0100);
			break;
		case MNY_CHG_100SUBYEN: // 100�~SUB�����ύX
			turi_dat.turi_in &= (~0x0008);
			break;
		case MNY_CHG_AFTER: // �ޑK�����ύX�̌㏈��
			if( (turi_dat.turi_in & 0x01fc) != 0x01fc )
			{	// ������ύX����
				turi_kan.turi_dat[0].sin_mai = turi_dat.coin_dsp[0];
				turi_kan.turi_dat[1].sin_mai = turi_dat.coin_dsp[1];
				turi_kan.turi_dat[2].sin_mai = turi_dat.coin_dsp[2];
				turi_kan.turi_dat[3].sin_mai = turi_dat.coin_dsp[3];
				turi_kan.turi_dat[0].ysin_mai = turi_dat.coin_dsp[4];
				turi_kan.turi_dat[1].ysin_mai = turi_dat.coin_dsp[5];
				turi_kan.turi_dat[2].ysin_mai = turi_dat.coin_dsp[6];
				memcpy( turi_dat.coin_sin, turi_dat.coin_dsp, sizeof(turi_dat.coin_sin) );
				turikan_prn(MNY_CHG_AFTER);
				cn_stat( 32, 0 ); // �ۗL�����̑��M�A������؉�
			}else{
				cn_stat( 2, 0 ); // ������؉�
			}
			turi_dat.turi_in = 0;
			break;
		case MNY_CTL_BEFORE: // �ޑK��[�̑O����
			memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) ); //���d�p�ޯ����� 
			turi_dat.turi_in = 0x01ff;	// ��[�����׸޾�� */
			memset( turi_dat.incount, 0, sizeof( turi_dat.incount ) ); // ���າݶ���(�ۗL�����ω���) 
			memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); // �o�າݶ���(�ۗL�����ω���) 
			memset( SFV_DAT.r_add08, 0, sizeof( SFV_DAT.r_add08 ) ); // ���ɖ����v�Z�p 
			memset( ryo_buf.out_coin, 0, sizeof(ryo_buf.out_coin) );
			memset( ryo_buf.in_coin, 0, sizeof(ryo_buf.in_coin) );
			memset( SFV_DAT.r_add0a, 0, sizeof( SFV_DAT.r_add0a ) ); // ���ɖ����v�Z�p ( ��[�����ɔ��������Z�o�Ɏg�p )
			for( i = 0; i < 4; i++ ){
				turi_kan.turi_dat[i].hojyu_safe = 0;					 // �ޑK��[�����ɔ��������̃N���A
			}

			if( CPrmSS[S_KAN][1] ){
				turikan_gen();
				turi_dat.coin_dsp[0] = turi_dat.dsp_ini[0] = turi_kan.turi_dat[0].gen_mai;
				turi_dat.coin_dsp[1] = turi_dat.dsp_ini[1] = turi_kan.turi_dat[1].gen_mai;
				turi_dat.coin_dsp[2] = turi_dat.dsp_ini[2] = turi_kan.turi_dat[2].gen_mai;
				turi_dat.coin_dsp[3] = turi_dat.dsp_ini[3] = turi_kan.turi_dat[3].gen_mai;
				turi_dat.coin_dsp[4] = turi_dat.dsp_ini[4] = turi_kan.turi_dat[0].ygen_mai;
				turi_dat.coin_dsp[5] = turi_dat.dsp_ini[5] = turi_kan.turi_dat[1].ygen_mai;
				turi_dat.coin_dsp[6] = turi_dat.dsp_ini[6] = turi_kan.turi_dat[2].ygen_mai;
				cn_stat( 21, 0 );	/* Coin Enable */
			}else{
				turi_dat.dsp_ini[0] = 0;
				turi_dat.dsp_ini[1] = 0;
				turi_dat.dsp_ini[2] = 0;
				turi_dat.dsp_ini[3] = 0;
				turi_dat.dsp_ini[4] = 0;
				turi_dat.dsp_ini[5] = 0;
				turi_dat.dsp_ini[6] = 0;
				cn_stat( 1, 0 );	// CREM ON, ������؋֎~
			}
			break;
		case MNY_INCOIN: // �ޑK��[���̓���
			if( OPECTL.CN_QSIG == (uchar)(MSGGETLOW(COIN_IH_EVT) )){
												/* ��������? */
				turi_dat.turi_in &= (~0x0001);	/* ��[���삠�辯� */
				for( i = 0; i < 4; i++ ){
									/* �ۗL�����̑�������\���ر�ɉ��Z���� */
					turi_dat.coin_dsp[i] = (ushort)bcdbin( CN_RDAT.r_dat07[i] );
				}
			}
			if( OPECTL.CN_QSIG == 5 ){	/* ��������? */
				cn_stat( 5, 0 ); /* �����p�� */
			}
			if( OPECTL.CN_QSIG == 1 ){	/* ��������? */
				turi_dat.turi_in &= (~0x0001);	/* ��[���삠�辯� */
			}
			break;
		case MNY_CTL_AFTER: // �ޑK��[�̌㏈��
							// CREM OFF���Ă���Ă΂��
			if(( turi_dat.turi_in & 0x0003 ) != 0x0003 ){
				if( CPrmSS[S_KAN][1] ){
					if( ( turi_dat.turi_in & 0x0001 ) == 0 )
					{	// ��������
						for( i = 0; i < 4; i++ )
						{
							turi_kan.turi_dat[i].hojyu = turi_dat.incount[i];
						}
						turiadd_hojyu_safe();					// �ޑK��[�����ɔ��������̉��Z
					}
					if( ( turi_dat.turi_in & 0x0002 ) == 0 )
					{	// �ނ荇�킹����
						for( i = 0; i < 4; i++ )
						{
							turi_kan.turi_dat[i].kyosei += turi_dat.outcount[i];
						}
					}
					turi_kan.turi_dat[0].sin_mai = turi_dat.coin_dsp[0];
					turi_kan.turi_dat[1].sin_mai = turi_dat.coin_dsp[1];
					turi_kan.turi_dat[2].sin_mai = turi_dat.coin_dsp[2];
					turi_kan.turi_dat[3].sin_mai = turi_dat.coin_dsp[3];
					turi_kan.turi_dat[0].ysin_mai = turi_dat.coin_dsp[4];
					turi_kan.turi_dat[1].ysin_mai = turi_dat.coin_dsp[5];
					turi_kan.turi_dat[2].ysin_mai = turi_dat.coin_dsp[6];

					turikan_prn( MNY_CTL_AFTER );	// �ޑK�Ǘ��� 
					kin = 1;
				}
				else{
					wopelg( OPLOG_TURIHOJU, 0, 0 );
				}
				cn_stat( 22, 0 );					// CREM OFF,CLR 
			}

			turi_dat.turi_in = 0;
			memcpy( &ckn[0], &SFV_DAT.safe_dt[0], 8 );
			turikan_sfv();
			if( ( kin == 1 )||
				( memcmp( &ckn[0], &SFV_DAT.safe_dt[0], 8 ) != 0 ) ){
				if( 4 == OPECTL.Mnt_mod ){			// ��[���ގg�p
					Make_Log_MnyMng( 20 );			// ���K�Ǘ����O�f�[�^�쐬
				}else{								// �ݒ�@����
					Make_Log_MnyMng( 30 );			// ���K�Ǘ����O�f�[�^�쐬
				}
			}
			break;
		case MNY_CTL_AUTO: // �ނ荇�킹�L�[����
			turi_dat.turi_in &= (~0x0002);	// AUTO���삠�辯� */
			wopelg( OPLOG_TURIAUTO, 0, 0 );	// AUTO����L�^ */
			turi_kan_f_exe_autostart = TURIKAN_AUTOSTART_NOEXE;// �ނ荇���J�n�Ŗ����s���
			cn_stat( 2, 0 );
			break;
		case MNY_CTL_AUTOSTART: // �ނ荇�킹�L�[������CREM OFF
			if( ( turi_dat.turi_in & 0x0001 ) == 0 )
			{	// ��������
				turikan_sfv();
				turiadd_hojyu_safe();					// �ޑK��[�����ɔ��������̉��Z
				memset( SFV_DAT.r_add08, 0, sizeof( SFV_DAT.r_add08 ) ); // ���ɖ����v�Z�p 
			}
			turi_kan_f_exe_autostart = TURIKAN_AUTOSTART_EXE;// �ނ荇���J�n�Ŏ��s���
			cn_stat( 34, 0 );		// �ނ荇�킹�J�n
			break;
		case MNY_CTL_AUTOCOMPLETE: // �ނ荇�킹�I��
			for( i=0; i<4; i++ ){
				if(turi_kan_f_exe_autostart == TURIKAN_AUTOSTART_EXE){	// �ނ荇�����s�ς�
					turi_dat.coin_dsp[i] = turi_dat.dsp_ini[i] = (ushort)bcdbin( CN_RDAT.r_dat07[i] );
				}
			}
			if(turi_kan_f_exe_autostart == TURIKAN_AUTOSTART_NOEXE){	// �ނ荇�������s
				turi_dat.turi_in |= 0x0002;							// ���K�Ǘ����v���󎚂��Ȃ�
			}
			cn_stat( 33, 0 );	// CREM ON, ������؉�
			break;
		case MNY_COIN_INVSTART: // �C���x���g������J�n
			if( CPrmSS[S_KAN][1] != 0 ){
				cn_stat( 8, 0 );	// CREM OFF+�ۗL�������M
			}
			else{
				cn_stat( 2, 0 );	// CREM OFF
			}
			turi_dat.turi_in = 0x01ff;	// ��[�����׸޾�� */
			memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); // �o�າݶ���(�ۗL�����ω���) 
			break;
		case MNY_COIN_INVCOMPLETE: // �C���x���g������I��
			wopelg( OPLOG_INVENTRY, 0, 0 );	// ������ؑ���L�^
			turi_dat.turi_in = 0;
			turikan_inventry( 0 );				// T�W�v�ɉ��Z
			break;
		default:
			break;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			��[�����ɔ��������v�Z
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			MATSUSHITA
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2009/02/23<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
void	turiadd_hojyu_safe(void)
{
	int		c, c2, i;
	
	for (i = 0; i < 4; i++) {
		if( SFV_DAT.r_add08[i] != 0 ){	/* ��������? */
			c = (int)turi_kan.turi_dat[i].gen_mai;			// �ޑK��[�J�n���̏����l(turi_dat.dsp_ini�͒ނ荇�킹��ɍX�V�����)
			c2 = (int)( bcdbin( SFV_DAT.r_dat07[i] ));
			c += SFV_DAT.r_add08[i];
			c -= SFV_DAT.r_add0a[i];
			c -= c2;
//NOTE�F�i��[�J�n���̖����{���������|�o�����������b�N�ۗ̕L�����j�̎����ɂɔ������ꂽ���ƂɂȂ�
//       ���b�N�ۗ̕L���������̏���̎��ɓ�������Ă����Z����Ȃ����ƂƁA�o�������͒ނ荇�����ɔ�������̂ŏ�L�̎��ƂȂ�܂�
			if( c > 0 ){
				turi_kan.turi_dat[i].hojyu_safe += c;
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| �ޑK��[��������                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turian_prn( prm )                                       |*/
/*| PARAMETER    : prm = 0:�ޑK��[����œ����Ȃ�                          |*/
/*|                    = 1:�ޑK��[����œ�������                          |*/
/*|                    = 2:������ؑ���                                     |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2001-12-25                                              |*/
/*| Update		 : ������ؑ���ł�����Ă��Ȃ� 2005-09-03 T.Hashimoto       |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/

void turikan_prn( short ope_kind )
{
	T_FrmTuriKan	FrmTuriKan;

	if( CPrmSS[S_KAN][1] == 0 ) return;

	turi_kan.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// �@�B��
	turi_kan.Kakari_no = OPECTL.Kakari_Num;								// �W���ԍ�set
	memcpy( &turi_kan.NowTime, &CLK_REC, sizeof( date_time_rec ) );		// ���ݎ���
	turikan_subtube_set();
	switch( ope_kind ){
		case MNY_CHG_AFTER: // ���ݖ����̕ύX
			wopelg( OPLOG_TURIHENKO, 0, 0 );
			Make_Log_MnyMng( 32 );				// ���K�Ǘ����O�f�[�^�쐬
			break;
		case MNY_CTL_AFTER: // �ޑK��[�̌㏈��
			wopelg( OPLOG_TURIHOJU, 0, 0 );
			Make_Log_MnyMng( 30 );				// ���K�Ǘ����O�f�[�^�쐬
			break;
		case MNY_COIN_CASETTE: // �R�C���J�Z�b�g��
			//wopelg( OPLOG_TURICASETTE );
			Make_Log_MnyMng( 32 );				// ���K�Ǘ����O�f�[�^�쐬
			break;
		default:
			break;
	}
	CountGet( TURIKAN_COUNT, &turi_kan.Oiban );				// �ǔ�

	FrmTuriKan.prn_kind = R_PRI;
	FrmTuriKan.prn_data = &turi_kan;

	CountUp(TURIKAN_COUNT);

	queset( PRNTCBNO, PREQ_TURIKAN, sizeof(T_FrmTuriKan), &FrmTuriKan  ); /* �ޑK�Ǘ���	*/
	turi_kan_f_defset_wait = TURIKAN_DEFAULT_WAIT;						//�󎚑҂��t���O�Z�b�g

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| �ޑK��[����I���ŋ��ɖ������Ă���                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_sfv                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_sfv( void )
{
	int	i, c;
	int c2;

	memcpy( SFV_DAT.safe_dt_wk, SFV_DAT.safe_dt, sizeof( SFV_DAT.safe_dt_wk ) );

	ac_flg.turi_syu = 10;

	for( i=0; i<4; i++ ){
		if( SFV_DAT.r_add08[i] != 0 ){	/* ��������? */
			c = (int)turi_dat.dsp_ini[i];
			c2 = (int)( bcdbin( SFV_DAT.r_dat07[i] ));
			if( c > c2 ){
				// ��������>���񖇐� �̏ꍇ(��������)
				c2 += 100;
				//�ޑK�Ǘ������s�ǱװѾ��(����/����)
				alm_chk( ALMMDL_SUB, ALARM_TURIKAN_REV, 2 );			// �װѓo�^
			}
			c += SFV_DAT.r_add08[i];
			c -= c2;
			if( c > 0 ){
				SFV_DAT.safe_dt[i] += c;
			}
		}
	}
	SFV_DAT.safe_cal_do = 0; /* ���ɖ����Z�o�ς� */
	ac_flg.turi_syu = 0;

	return;
}

// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
extern	const	date_time_rec	nulldate;
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �ޑK�Ǘ��ر�N���A���菈��																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: Check_turikan_clr( void )														   |*/
/*| PARAMETER	: 																				   |*/
/*| RETURN VALUE: char	: 0 = �N���A�s��														   |*/
/*|						  1 = �N���A��															   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
char	Check_turikan_clr( void )
{
	TURI_KAN	*dsy;

	dsy = &turi_kan;													/* ���ݏW�v						*/

	//�C���ݏW�v�̍���W�v������0�N���A����Ă��邩�m�F
	if (memcmp(&dsy->NowTime, &nulldate, sizeof(dsy->NowTime)) == 0) {
		// ���ݏW�v�������Ȃ����N���A����Ă���
		wmonlg(OPMON_TURIKAN_IRGCLR1, 0, 0);						// ���j�^�o�^
		return 0;
	}

	//�D���݂̒ǔԁA�W�����A�@�B����0�N���A����Ă���ꍇ�̓N���A�L�����Z��
	if (( CountSel( &dsy->Oiban ) == 0) && 
		( dsy->Kakari_no == 0 ) && 
		( dsy->Kikai_no == 0 )){
		wmonlg(OPMON_TURIKAN_IRGCLR2, 0, 0);						// ���j�^�o�^
		return 0;
	}

	return 1;
}
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �ޑK�Ǘ��ر�̍X�V																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: turikan_clr( void )															   |*/
/*| PARAMETER	: void	:																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimoto(AMANO)															   |*/
/*| Date		: 2001-12-25																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void turikan_clr( void )
{
																	/*								*/
	ac_flg.syusyu = 52;												/* 52:�󎚊���					*/
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		turi_kan.CenterSeqNo = RAU_GetCenterSeqNo(RAU_SEQNO_CHANGE);
	}
																	/*								*/
	Log_regist( LOG_MONEYMANAGE );									/* ���K�Ǘ����o�^				*/
																	/*								*/
	if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		RAU_UpdateCenterSeqNo(RAU_SEQNO_CHANGE);
	}
																	/*								*/
	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );				/*								*/
																	/*								*/
	ac_flg.syusyu = 55;												/* 55:�ޑK�Ǘ��W�v��ܰ��ر�֓]������*/
	turikan_clr_sub2();												/*								*/
																	/*								*/
	ac_flg.syusyu = 56;												/* 56:���K�Ǘ��W�v�ر����		*/
	turikan_clr_sub();												/* T�W�v�֒ޑK��[�A�������o�����Z */
																	/*								*/
	ac_flg.syusyu = 58;												/* 58:��[�ް���T�W�v�։��Z�I��	*/

	turi_kan_f_defset_wait = TURIKAN_DEFAULT_NOWAIT;//�f�t�H���g�Z�b�g�҂��t���O�N���A

	return;
}

void turikan_clr_sub2( void )
{
	int	i;															/*								*/
																	/*								*/
	memset( &turi_kan, 0, sizeof( TURI_KAN ) );						/* �ޑK�Ǘ��ر�ر				*/
																	/*								*/
	for( i = 0; i < 4; i++ ){										/*								*/
		turi_kan.turi_dat[i].gen_mai = turi_kwk.turi_dat[i].sin_mai;/* ���ݕۗL����<-�V�K�ݒ薇��	*/
		turi_kan.turi_dat[i].zen_mai = turi_kwk.turi_dat[i].sin_mai;/* �O��ۗL����<-�V�K�ݒ薇��	*/
		turi_kan.turi_dat[i].ygen_mai = turi_kwk.turi_dat[i].ysin_mai;/* �O��ۗL����<-�V�K�ݒ薇��	*/
		turi_kan.turi_dat[i].yzen_mai = turi_kwk.turi_dat[i].ysin_mai;/* �O��ۗL����<-�V�K�ݒ薇��	*/
	}																/*								*/
	memcpy( &turi_kan.OldTime, &turi_kwk.NowTime, sizeof( date_time_rec ) );	/*					*/

	return;
}																	/* �O��W�v����<-����W�v����	*/

void turikan_clr_sub( void )
{
	int		i;														/*								*/
	SYUKEI	*ts, *ws;												/*								*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
	ws = &wksky;													/*								*/
	memcpy( ws, ts, sizeof( SYUKEI ) );								/* �s�W�v��ܰ��ر�ցi��d�΍�j	*/
																	/*								*/
	ac_flg.syusyu = 57;												/* 57:��[�ް���T�W�v�։��Z�O	*/
																	/*								*/
	ts->hoj[0] += ( (unsigned long)turi_kwk.turi_dat[0].hojyu );	/* �ޑK��[���z  10�~�g�p�z		*/
	ts->hoj[1] += ( (unsigned long)turi_kwk.turi_dat[1].hojyu );	/* �ޑK��[���z  50�~�g�p�z		*/
	ts->hoj[2] += ( (unsigned long)turi_kwk.turi_dat[2].hojyu );	/* �ޑK��[���z 100�~�g�p�z		*/
	ts->hoj[3] += ( (unsigned long)turi_kwk.turi_dat[3].hojyu );	/* �ޑK��[���z 500�~�g�p�z		*/
	ts->hojyu = 0L;													/*								*/
	for( i = 0; i < 4; i++ ){										/*								*/
		ts->hojyu += (ts->hoj[i] * coin_vl[i]);						/* �ޑK��[���z	���z			*/
	}																/*								*/
																	/*								*/
	// �ȉ��͒ނ荇�킹�ŕ����o���������p
	ts->kyo[0] += ( (unsigned long)turi_dat.outcount[0] );				/* �������o���z  10�~�g�p�z		*/
	ts->kyo[1] += ( (unsigned long)turi_dat.outcount[1] );				/* �������o���z  50�~�g�p�z		*/
	ts->kyo[2] += ( (unsigned long)turi_dat.outcount[2] );				/* �������o���z 100�~�g�p�z		*/
	ts->kyo[3] += ( (unsigned long)turi_dat.outcount[3] );				/* �������o���z 500�~�g�p�z		*/
	memset( turi_dat.outcount, 0, sizeof( turi_dat.outcount ) ); 	// �o�າݶ���(�ۗL�����ω���) 
	ts->kyosei = 0L;												/*								*/
	for( i = 0; i < 4; i++ ){										/*								*/
		ts->kyosei += (ts->kyo[i] * coin_vl[i]);					/* �������o���z	���z			*/
	}																/*								*/

	return;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ������ؖ������s�W�v�ɉ��Z																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: turikan_inventry( prm )														   |*/
/*| PARAMETER	: short	prm=0:հ�ް���Ұ��̲�����؁A=1:ү����݂̲������							   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimoto(AMANO)															   |*/
/*| Date		: 2005-12-25																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void turikan_inventry( short prm )
{
	int	i;
	SYUKEI	*ts, *ws;

	ts = &sky.tsyuk;
	ws = &wksky;
	memcpy( ws, ts, sizeof( SYUKEI ) );								/* �s�W�v��ܰ��ر�ցi��d�΍�j	*/
	memcpy( &turi_kwk, &turi_kan, sizeof( TURI_KAN ) );

	if( prm == 0 ){

		// հ�ް���Ұ�����ɂ��������
		ac_flg.syusyu = 60;											/* 60:�������o������T�W�v�։��Z�O	*/

		for( i = 0; i < 4; i++ ){
			turi_kan.turi_dat[i].kyosei += turi_dat.outcount[i];	/* �������o���z  10�`500�~�g�p�z	*/
		}
		turi_kan.turi_dat[0].ykyosei += turi_dat.outcount[4];		/* �������o���z  10�~�g�p�z		*/
		turi_kan.turi_dat[1].ykyosei += turi_dat.outcount[5];		/* �������o���z 50�~�g�p�z		*/
		turi_kan.turi_dat[2].ykyosei += turi_dat.outcount[6];		/* �������o���z 100�~�g�p�z		*/

		for( i = 0; i < 4; i++ ){
			ts->kyo[i] += ( (unsigned long)turi_dat.outcount[i] );	/* �������o���z  10�~�g�p�z		*/
		}
		ts->kyo[0] += ( (unsigned long)turi_dat.outcount[4] );		/* �������o���z  10�~�\�~�g�p�z */
		ts->kyo[1] += ( (unsigned long)turi_dat.outcount[5] );		/* �������o���z 50�~�\�~�g�p�z */
		ts->kyo[2] += ( (unsigned long)turi_dat.outcount[6] );		/* �������o���z 100�~�\�~�g�p�z */
		ts->kyosei = 0L;											/*								*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->kyosei += (ts->kyo[i] * coin_vl[i]);				/* �������o���z	���z			*/
		}															/*								*/

		ac_flg.syusyu = 62;											/* 58:�������o������T�W�v�։��Z��	*/

	}else{

		// ү��������݂ɂ��������
		ac_flg.syusyu = 61;											/* 58:�������o������T�W�v�։��Z�O	*/

		for( i = 0; i < 4; i++ ){
			turi_kan.turi_dat[i].kyosei += turi_dat.forceout[i];	/* �������o���z  10�`500�~�g�p�z	*/
		}
		turi_kan.turi_dat[0].ykyosei += turi_dat.forceout[4];		/* �������o���z 10�~�\�~�g�p�z */
		turi_kan.turi_dat[1].ykyosei += turi_dat.outcount[5];		/* �������o���z 50�~�g�p�z		*/
		turi_kan.turi_dat[2].ykyosei += turi_dat.outcount[6];		/* �������o���z 100�~�g�p�z		*/

		for( i = 0; i < 4; i++ ){
			ts->kyo[i] += turi_dat.forceout[i];						/* �������o���z  10�`500�~�g�p�z */
		}
		ts->kyo[0] += turi_dat.forceout[4];							/* �������o���z  10�~�\�~�g�p�z	*/
		ts->kyo[1] += turi_dat.forceout[5];							/* �������o���z 50�~�\�~�g�p�z */
		ts->kyo[2] += turi_dat.forceout[6];							/* �������o���z 100�~�\�~�g�p�z */
		ts->kyosei = 0L;											/*								*/
		for( i = 0; i < 4; i++ ){									/*								*/
			ts->kyosei += (ts->kyo[i] * coin_vl[i]);				/* �������o���z	���z			*/
		}															/*								*/

		ac_flg.syusyu = 62;											/* 58:�������o������T�W�v�։��Z��	*/
	}
	turikan_gen();

	Make_Log_MnyMng( 31 );											/* ���K�Ǘ����O�f�[�^�쐬		*/
	Log_regist( LOG_MONEYMANAGE_NT );								/* ���K�Ǘ����O�o�^				*/

}

/*[]----------------------------------------------------------------------[]*/
/*| �ޑK�Ǘ��̕��d����                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turikan_fuk                                             |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2002-01-31                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
void turikan_fuk( void )
{
	int	i;

	if( turi_dat.turi_in != 0 ){ /* �ޕ�[���쒆? */

		memcpy( &turi_kan, &turi_kwk, sizeof( TURI_KAN ) );

		if( ( turi_dat.turi_in & 0x0003 ) != 0x0003 ){ /* ���o������ */
			if( ( turi_dat.turi_in & 0x0001 ) == 0 ){ /* ���o������ */
				turikan_sfv(); /* ���ɖ����Z�o */
			}
			if( CPrmSS[S_KAN][1] ){
				// hojyu, kyosei, kyosei_safe�̍X�V
				if( ( turi_dat.turi_in & 0x0001 ) == 0 )
				{	// ��������
					for( i = 0; i < 4; i++ )
					{
						turi_kan.turi_dat[i].hojyu += turi_dat.incount[i];
					}
				}
				if( ( turi_dat.turi_in & 0x0002 ) == 0 )
				{	// �ނ荇�킹����
					for( i = 0; i < 4; i++ )
					{
						turi_kan.turi_dat[i].kyosei += turi_dat.outcount[i];
					}
				}
				// sin_mai�X�V
				for( i = 0; i < 4; i++){
					turi_kan.turi_dat[i].sin_mai = (ushort)bcdbin( SFV_DAT.r_dat07[i] );
				}
				turiadd_hojyu_safe();					// �ޑK��[�����ɔ��������̉��Z
			}
		}
	}
	turi_dat.turi_in = 0;

	switch( ac_flg.turi_syu ){ /* �ޑK�Ǘ��X�V�� */
		case 1: /* ���Ē����̍X�V */
		case 2: /* ���ۗL�����Z�o�� */
			memcpy( &turi_kan, &turi_kwk, sizeof( TURI_KAN ) );
			ac_flg.turi_syu = 0;
			break;
		case 10: /* ���ɖ����Z�o�� */
			memcpy( SFV_DAT.safe_dt, SFV_DAT.safe_dt_wk, sizeof( SFV_DAT.safe_dt ) );
			turikan_sfv();
			break;
		case 20: /* ���ɖ�����ܰ��ر�֓]������ */
			memcpy( SFV_DAT.safe_dt, SFV_DAT.safe_dt_wk, sizeof( SFV_DAT.safe_dt ) );	/* �ޔ��ް������ɖ߂� */
			safecl( 7 );	/* ���ɖ����Z�o */
			ac_flg.turi_syu = 0;
			break;
		case 22: /* �ޑK�Ǘ��ر��ܰ��֓]������ */
			memcpy( &turi_kan, &turi_kwk, sizeof( TURI_KAN ) );	/* �ޔ��ް������ɖ߂� */
			ac_flg.turi_syu = 21;
			// no break
		case 21: /* ���ɏW�v���� */
			turikan_pay();				/* ���Z�������ɓ��o�������i�z�̂݁j���Ă��� */
			SFV_DAT.safe_cal_do = 0;	/* ���ɖ����v�Z�w��ؾ�� */
			ac_flg.turi_syu = 0;
			break;
		default:
			ac_flg.turi_syu = 0;
			break;
	}

	return;
}
/*[]----------------------------------------------------------------------[]*/
/*| �ޑK�Ǘ��\�~���Z�b�g                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : turian_subtube_set( void )                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2014-10-28                                              |*/
/*| Update		 :                                                         |*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]*/
void turikan_subtube_set( void )
{
	int	i, ret;

	// NOTE: turi_kan.sub_tube �ɂ͋��z�̏��������ɁA���ʃo�C�g����
	//       �\�~�P�A�\�~�Q�A�Ƃ��������Ŋi�[�����i���݂��镪�����i�[�j�B
	//       ���ׁ̈A���ۂ̑����ʒu�i�T�u�A�⏕�j�Ƃ͕K��������v���Ȃ��B

	turi_kan.sub_tube = 0;

	for( i=0; i<3; i++ ){
		ret = subtube_use_check( i );	// �e����ɑ΂���\�~�̊����󋵂��擾����
		switch( ret ){
		case 1:	// �\�~�P
			turi_kan.sub_tube |= (0x0001<<i);
			break;
		case 2:	// �\�~�Q
			turi_kan.sub_tube |= (0x0001<<(8+i));
			break;
		default:
			break;
		}
	}
}
