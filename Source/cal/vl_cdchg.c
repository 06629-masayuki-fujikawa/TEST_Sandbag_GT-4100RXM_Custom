/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���ް�����																					   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"mem_def.h"
#include	"flp_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mif.h"
#include	"LKmain.h"
#include	"prm_tbl.h"
#include	"pri_def.h"


static char	datenearend( char *edate );

// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�������^���������ް�����(�����v�Z�p�ɕϊ�����)
//[]----------------------------------------------------------------------[]
///	@param[in]		f_Button 1=�������݁A0=������
///	@param[in]		mag		���C�����ް�
///	@return			ret		0 = OK
///	@author			R.Hara
///	@note			���� 2Bh�ɑΉ�	
///	@attention		None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2005/10/01<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
int		vl_funchg( char f_Button, m_gtticstp *mag )
{
	cr_dat_n = RID_FUN/*0x2B*/;										// �����
	vl_now = V_FUN;
	memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );

	car_in.mon   = vl_tik.tki;										//   ��
	car_in.day   = vl_tik.hii;										//   ��
	car_in.hour  = vl_tik.jii;										//   ��
	car_in.min   = vl_tik.fun;										//   ��
	car_in.year  = CLK_REC.year;
	if( (car_in.mon > CLK_REC.mont) ||								/* ���Ԍ��������݌����H */
		((car_in.mon == CLK_REC.mont) && (car_in.day > CLK_REC.date)) ){
		car_in.year--;
	}
	car_in.week  = (char)youbiget( car_in.year ,					//     �j��
							(short)car_in.mon  ,
							(short)car_in.day  );
	return	0;
}
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���Ԍ��ް�����(�����v�Z�p�ɕϊ�����)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_carchg( no, chg_flg )														   |*/
/*| PARAMETER	: no	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*|				: chg_flg	; �ϊ����															   |*/
/*|							;   0:�ʏ�															   |*/
/*|							;   1,2:�����v�Z�V�~�����[�^										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	vl_carchg( ushort no , uchar chg_flg )
{
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	flp_com	*lk;
//	ulong	posi;
//	uchar	i;
//	ushort	wari_main = 0;
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

	if(( no >= 1 && no <= LOCK_MAX )||( no == 0xffff )){		// ���Ԉʒu�ԍ�����(1�`LOCK_MAX or ������Z)

		memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );		// ���Ԍ��ް�(vl_tik)������(0�ر)
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		if( no == 0xffff ){										// ������Z�⍇����?
//			lk = &LOCKMULTI.lock_mlt;
//			vl_tik.syu = (char)lk->ryo_syu;						// ��ʾ��(������Z�p)
//			Flap_Sub_Num = 10;									// ������Z
//		}else{
//			lk = &FLAPDT.flp_data[no-1];
//			vl_tik.syu = LockInfo[no-1].ryo_syu;				// ��ʾ��(���Ԉʒu�ݒ薈)
//		}
//
//		if (chg_flg != 0) {										// �����v�Z�V�~�����[�^
//		// �����v�Z�V�~�����[�^���͒��~�f�[�^���Q�Ƃ��Ȃ�
//			cr_dat_n = 0x24;									// ��ID���޾��(���Z�O��)
//			vl_now = V_CHM;										// �Ǎ��݌���ʾ��(���Z�O��)
//		}
//		else 
//		if( lk->bk_syu ){										// ���(���~,�C���p)
//			cr_dat_n = 0x26;									// ��ID���޾��(���Z���~��)
//			vl_now = V_CHS;										// �Ǎ��݌���ʾ��(���Z���~��)
//
//			vl_tik.syu = (uchar)lk->bk_syu;						// ��ʾ��(���Z���~���ۑ��ް�)
//
//			card_use[USE_SVC] = (uchar)lk->bk_wmai;				// �g�p����(���~,�C���p)
//			vl_tik.wari = lk->bk_wari;							// �������z(���~,�C���p)
//			vl_tik.time = lk->bk_time;							// �������Ԑ�(���~,�C���p)
//			vl_tik.pst = (uchar)lk->bk_pst;						// ��������(���~,�C���p)
//		}else{
//			// ���Z��Ԃ��u���Z���~�v�ȊO�̏ꍇ
//			cr_dat_n = 0x24;									// ��ID���޾��(���Z�O��)
//			vl_now = V_CHM;										// �Ǎ��݌���ʾ��(���Z�O��)
//		}
		cr_dat_n = 0x24;									// ��ID���޾��(���Z�O��)
		vl_now = V_CHM;										// �Ǎ��݌���ʾ��(���Z�O��)

		vl_tik.syu = syashu;
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

		vl_tik.chk = 0;											// ���Z�O�����

		vl_tik.cno = KIHON_PKNO;								// ���ԏꇂ���(��{���ԏꇂ)
		vl_tik.ckk = (short)CPrmSS[S_PAY][2];					// �����@No.
		vl_tik.hno = 0;											// ����No.
		vl_tik.mno = 0;											// �X�����

		vl_tik.tki = car_in_f.mon;								// ���Ԍ����
		vl_tik.hii = car_in_f.day;								// ���ԓ����
		vl_tik.jii = car_in_f.hour;								// ���Ԏ����
		vl_tik.fun = car_in_f.min;					 			// ���ԕ����
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		syashu       = vl_tik.syu	;							// �Ԏ�
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		hzuk.y = car_ot_f.year;
		hzuk.m = car_ot_f.mon;
		hzuk.d = car_ot_f.day;

		hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

		jikn.t = car_ot_f.hour;
		jikn.m = car_ot_f.min;

		jikn.s = 0;

		car_in.year  = car_in_f.year;							// ���ԔN
		car_in.mon   = vl_tik.tki;								//     ��
		car_in.day   = vl_tik.hii;								//     ��
		car_in.hour  = vl_tik.jii;								//     ��
		car_in.min   = vl_tik.fun;								//     ��
		car_in.week  = (char)youbiget( car_in.year ,			//     �j��
								(short)car_in.mon  ,
								(short)car_in.day  );
		car_ot.year = hzuk.y;									// �o�ɔN
		car_ot.mon  = hzuk.m;									//     ��
		car_ot.day  = hzuk.d;									//     ��
		car_ot.week = hzuk.w;									//     �j��
		car_ot.hour = jikn.t;									//     ��
		car_ot.min  = jikn.m;									//     ��
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		if (chg_flg != 0) {										// �����v�Z�V�~�����[�^
//		// �����v�Z�V�~�����[�^���͒��~�f�[�^���Q�Ƃ��Ȃ�
//			Flap_Sub_Flg = 0;
//		}
//		else 
//		if( lk->bk_syu ){										// ���(���~,�C���p)
//			if( no == 0xffff ){									// ������Z
//				//lock_multi���ڍג��~�o�b�t�@�Ɋi�[����B
//				wari_main = 0;
//				Flap_Sub_Flg = 0;								//�׸޸ر
//				memset(&FLAPDT_SUB[10],0,sizeof(flp_com_sub));	//�ر
//				memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
//				FLAPDT_SUB[10].TInTime.Year=LOCKMULTI.lock_mlt.year;
//				FLAPDT_SUB[10].TInTime.Mon=LOCKMULTI.lock_mlt.mont;
//				FLAPDT_SUB[10].TInTime.Day=LOCKMULTI.lock_mlt.date;
//				FLAPDT_SUB[10].TInTime.Hour=LOCKMULTI.lock_mlt.hour;
//				FLAPDT_SUB[10].TInTime.Min=LOCKMULTI.lock_mlt.minu;
//				memcpy(&FLAPDT_SUB[10].sev_tik,&LOCKMULTI.sev_tik_mlt,sizeof(uchar)*15);			//���޽�������i�[
//				memcpy(&FLAPDT_SUB[10].kake_data,&LOCKMULTI.kake_data_mlt,sizeof(kake_tiket)*5);	//�|���������i�[
//				FLAPDT_SUB[10].ppc_chusi_ryo = LOCKMULTI.ppc_chusi_ryo_mlt;							//�v���y�C�h���z�i�[
//				FLAPDT_SUB[10].syu = (uchar)LOCKMULTI.lock_mlt.bk_syu;								//������ʊi�[
//				Flap_Sub_Num = 10;																	//������Z�p�i�[�ʒu
//				for(i=0;i<15;i++){
//					if(FLAPDT_SUB[10].sev_tik[i] != 0){					//���޽������
//						Flap_Sub_Flg = 1;								//B�ر�g�p�t���OON
//						wari_main += FLAPDT_SUB[10].sev_tik[i];			//�g�p�������Z
//					}
//				}
//				for(i=0;i<5;i++){
//					if(FLAPDT_SUB[10].kake_data[i].maisuu != 0){			//�|��������
//						Flap_Sub_Flg = 1;									//B�ر�g�p�t���OON
//						wari_main += FLAPDT_SUB[10].kake_data[i].maisuu;	//�g�p�������Z
//					}
//				}
//				if(wari_main != LOCKMULTI.lock_mlt.bk_wmai){		//�g�p����������Ŗ�����
//					Flap_Sub_Flg = 0;								//�׸޸ر
//				}
//				if((FLAPDT_SUB[10].ppc_chusi_ryo)&&(LOCKMULTI.lock_mlt.bk_wari)){				//�v���y�C�h����
//					if(LOCKMULTI.lock_mlt.bk_wari >= FLAPDT_SUB[10].ppc_chusi_ryo){
//						Flap_Sub_Flg = 1;														//B�ر�g�p�t���OON
//					}else{
//						Flap_Sub_Flg = 0;								//�׸޸ر
//					}
//				}
//			}else{
//				posi = LockInfo[no-1].posi;						//�����ԍ���蒓�Ԉʒu���擾
//				vl_cyushiset( posi );
//			}
//		}else{
//			Flap_Sub_Flg = 0;
//		}
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	}
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���������(�����v�Z�p�ɕϊ�����)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_paschg( mag )																   |*/
/*| PARAMETER	: mag	; ���Cذ�ް��M�ް�														   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static short 	pno_dt[8] = {
	0x0000, 0x0200, 0x0100, 0x0300, 0x0080,0x0280, 0x0180, 0x0380
};

short	vl_paschg( void )
{
	short	ret = 0;
	char	c_prm;
	short	cnt;

	cr_dat_n = RID_APS;										// ID Code(1AH)
	memset( &tsn_tki, 0x00, sizeof( struct TSN_TKI) );

	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CRD_DAT.PAS.pno == CPrmSS[S_SYS][cnt]){
		tsn_tki.pkno = cnt-1;								// ���ԏꇂ��ʾ��
		break;
		}
	}
	tsn_tki.kind = (char)(CRD_DAT.PAS.knd);						// ��ʾ��
	tsn_tki.code = CRD_DAT.PAS.cod;								// �l���޾��

	c_prm = (char)prm_get( COM_PRM,S_PAS,						// �������ݒ�Get
			(short)(5+10*(CRD_DAT.PAS.knd-1)),1,2 );
	if( c_prm == 1 || c_prm == 2 ){								// ������or�J�n��������
		tsn_tki.data[0] = 90;									// 1990/1/1���
		tsn_tki.data[1] = 1;
		tsn_tki.data[2] = 1;
	}else{
		memcpy( &tsn_tki.data[0], &CRD_DAT.PAS.std_end[0], 3 );
	}

	if( c_prm == 1 || c_prm == 3 ){								// ������or�I����������
		tsn_tki.data[3] = 79;									// 2079/12/31���
		tsn_tki.data[4] = 12;
		tsn_tki.data[5] = 31;
	}else{
		memcpy( &tsn_tki.data[3], &CRD_DAT.PAS.std_end[3], 3 );
	}

	tsn_tki.status = (char)CRD_DAT.PAS.sts;
	memcpy( tsn_tki.sttim, CRD_DAT.PAS.trz, 4 );

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;

	if( c_prm == 1 || c_prm == 3 ){								// ������or�I����������
		;
	}else{
		if( !datenearend( &tsn_tki.data[3] ) ){					// �����؂�ԋ�����
			ret = 1;											// �����؂�ԋ�
		}
	}

	return( ret );
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| Mifare���������(�����v�Z�p�ɕϊ�����)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_mifpaschg( void )															   |*/
/*| PARAMETER	: void	;																		   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
short	vl_mifpaschg( void )
{
	short	ret = 0;
	char	c_prm;
	short	cnt;

	cr_dat_n = RID_APS;											// APS����Ƃ���(1AH)
	memset( &tsn_tki, 0x00, sizeof( struct TSN_TKI ) );

	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(MifCard.pk_no == CPrmSS[S_SYS][cnt]){
		tsn_tki.pkno = cnt-1;									// ���ԏꇂ���
		break;
		}
	}
	tsn_tki.kind = (char)(MifCard.pas_knd);						// ��ʾ��
	tsn_tki.code = MifCard.pas_id;								// �l���޾��

	c_prm = (char)prm_get( COM_PRM,S_PAS,						// �������ݒ�Get
			(short)(5+10*(MifCard.pas_knd-1)),1,2 );
	if( c_prm == 1 || c_prm == 2 ){								// ������or�J�n��������
		tsn_tki.data[0] = 90;									// 1990/1/1���
		tsn_tki.data[1] = 1;
		tsn_tki.data[2] = 1;
	}else{
		tsn_tki.data[0] =										// �J�n�N
				(char)( MifCard.limit_s.Year % 100 );
		tsn_tki.data[1] = MifCard.limit_s.Mon;					// �J�n��
		tsn_tki.data[2] = MifCard.limit_s.Day;					// �J�n��
	}

	if( c_prm == 1 || c_prm == 3 ){								// ������or�I����������
		tsn_tki.data[3] = 79;									// 2079/12/31���
		tsn_tki.data[4] = 12;
		tsn_tki.data[5] = 31;
	}else{
		tsn_tki.data[3] =										// �I���N
				(char)( MifCard.limit_e.Year % 100 );
		tsn_tki.data[4] = MifCard.limit_e.Mon;					// �I����
		tsn_tki.data[5] = MifCard.limit_e.Day;					// �I����
	}
	
	tsn_tki.status = (uchar)(MifCard.io_stat & 0x0F);

	memcpy( tsn_tki.sttim, &MifCard.ext_tm.Mon, 4 );			// ������������

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;

	if( c_prm == 1 || c_prm == 3 ){								// ������or�I����������
		;
	}else{
		if( !datenearend( &tsn_tki.data[3] ) ){					// �����؂�ԋ�����
			ret = 1;											// �����؂�ԋ�
		}
	}

	return( ret );
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���޽������(�����v�Z�p�ɕϊ�����)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_svschk( mag )																   |*/
/*| PARAMETER	: mag	; ���Cذ�ް��M�ް�														   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	vl_svschk( m_gtservic *mag )
{

	long	temp = 0;
	short	cnt;

	cr_dat_n = 0x2d;

	memset( &vl_svs, 0x00, sizeof( struct VL_SVS ) );

	temp = (long)mag->servic.svc_pno[0] + 						// ���ԏ�No.
						(long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GT�t�H�[�}�b�g
		temp |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17�i�[
	}
	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CPrmSS[S_SYS][cnt] == temp){
			vl_svs.pkno = cnt-1;									// ���ԏ�No.���
			break;
		}
	}

	vl_svs.sno  = (char)(mag->servic.svc_pno[1] & 0x0f);				// ��ʾ��

	vl_svs.mno  = (short)mag->servic.svc_sno[1] + 						// �XNo.
						(((short)mag->servic.svc_sno[0])<<7);

	vl_svs.ymd[0] = mag->servic.svc_sta[0];							// �L������(�J�n)
	vl_svs.ymd[1] = mag->servic.svc_sta[1];
	vl_svs.ymd[2] = mag->servic.svc_sta[2];

	vl_svs.ymd[3] = mag->servic.svc_end[0];							// �L������(�I��)
	vl_svs.ymd[4] = mag->servic.svc_end[1];
	vl_svs.ymd[5] = mag->servic.svc_end[2];

	vl_svs.sts = mag->servic.svc_sts;									// �������set

	return( 0 );
}

// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) G.So IC�N���W�b�g�Ή�
////[]----------------------------------------------------------------------[]
/////	@brief			���E��������(�����v�Z�p�ɃT�[�r�X(�|��)���`���ϊ�����)
////[]----------------------------------------------------------------------[]
/////	@param[in]		mno		�XNo.�^�T�[�r�X����
/////	@return			ret		0/-1
/////	@author			G.So
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/01/25<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//short	vl_sousai( ushort mno )
//{
//	cr_dat_n = 0x2d;												// �T�[�r�X(�|��)��
//	vl_now = V_SAK;													// �T�[�r�X��
//
//	memset( &vl_svs, 0x00, sizeof( struct VL_SVS ) );
//	vl_svs.pkno = KIHON_PKNO;										// ���ԏ���=��{���ԏ�No.
//	if (mno >= 1 && mno <= 100) {
//	// �|����
//		vl_svs.mno  = mno;											// �XNo.
//	}
//	else if (mno >= 9001 && mno <= 9015) {
//	// �T�[�r�X��
//		vl_svs.sno  = (char)(mno - 9000);							// �T�[�r�X����
//	}
//	else {
//		vl_svs.sno  = 1;											// �͈͊O�̓T�[�r�X���`�Ƃ݂Ȃ�
//	}
//
//	return( 0 );
//}
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��


/*[]----------------------------------------------------------------------------------------------[]*/
/*| ����߲�޶��ޏ���(�����v�Z�p�ɕϊ�����)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_prechg( mag )																   |*/
/*| PARAMETER	: mag	; ���Cذ�ް��M�ް�														   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	vl_prechg( m_gtprepid *mag )
{
	short	wk, i ,cnt;
	long temp;

	cr_dat_n = mag->prepid.pre_idc;

	memset( &tsn_prp, 0x00, sizeof( struct TSN_PRP ) );

	if(mag->magformat.type == 1){
		temp  =  (long)mag->prepid.pre_pno[0] & 0x0000003F;
		temp |= ((long)mag->prepid.pre_pno[1] & 0x0000003F) << 6;
		temp |= ((long)mag->prepid.pre_pno[2] & 0x0000003F) << 12;
	}else{
		temp = astoin( mag->prepid.pre_pno, 2 );
		temp += ( mag->prepid.pre_pno[2]&0x0f ) * 100;
	}
	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CPrmSS[S_SYS][cnt] == temp){
			tsn_prp.pakno = cnt-1;									//���ԏ�No���
			break;
		}
	}

	tsn_prp.hanno = (char)(astoin( mag->prepid.pre_mno, 2 ));			// �̔��@�����

	wk = (short)mag->prepid.pre_amo;
	if( wk < 0x40 ){											// "0"-"9"?
		wk -= 0x31;
		wk += 1;
	}else if( wk < 0x5b ){										// "A"-"Z"?
		wk -= 0x41;
		wk += 10;
	}else{														// "a"-"z"
		wk -= 0x61;
		wk += 36;
	}
	tsn_prp.hangk = wk * 1000l;									// �̔����z���
	tsn_prp.zan = (long)astoinl( mag->prepid.pre_ram, 5 );				// �c�z���
	tsn_prp.rno= (long)astoinl( mag->prepid.pre_cno, 5 );				// ���އ�

	tsn_prp.kid = (char)( mag->prepid.pre_pno[3] - 0x30 );

	for( i = 0; i < 3; i++ )
	{
		tsn_prp.ymd[i] = (char)(astoin( &mag->prepid.pre_sta[i*2],2 ));
	}

	tsn_prp.han = mag->prepid.pre_amo;
	tsn_prp.sei = PRC_GENDOGAKU_MIN;				 					// ���x�z���
	tsn_prp.hansam = mag->prepid.pre_sum;								// Check Sum

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;

	return( 0 );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �񐔌�����(�����v�Z�p�ɕϊ�����)															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_kaschg( mag )																   |*/
/*| PARAMETER	: mag	; ���Cذ�ް��M�ް�														   |*/
/*| RETURN VALUE: 100=�����؂�ԋ߁A0=OK�i�ԋ߂łȂ��j											   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

short	vl_kaschg( m_gtservic *mag )
{
	short	ret = 0;
	long	temp = 0;
	short	cnt;

	cr_dat_n = 0x2c;

	memset( &vl_kas, 0x00, sizeof( struct VL_KAS ) );

	temp = (long)mag->servic.svc_pno[0] + 						// ���ԏ�No.
						(long)pno_dt[mag->servic.svc_pno[1]>>4];
	if(mag->magformat.type == 1){//GT�t�H�[�}�b�g
		temp |= ((long)mag->magformat.ex_pkno & 0x000000ff)<< 10L;	//P10-P17�i�[
	}
	for(cnt = 1 ; cnt < 5 ; cnt++){
		if(CPrmSS[S_SYS][cnt] == temp){
			vl_kas.pkno = cnt-1;									//���ԏ�No���
			break;
		}
	}

	vl_kas.rim  = (char)(mag->servic.svc_pno[1] & 0x0f);				// �����x�����

	vl_kas.tnk  = (short)mag->servic.svc_sno[1] + 						// �P�ʋ��z
						(((short)mag->servic.svc_sno[0])<<7);
	vl_kas.nno = mag->servic.svc_sts;									// �c��


	vl_kas.data[0] = mag->servic.svc_sta[0];							// �L������(�J�n)
	vl_kas.data[1] = mag->servic.svc_sta[1];
	vl_kas.data[2] = mag->servic.svc_sta[2];

	vl_kas.data[3] = mag->servic.svc_end[0];							// �L������(�I��)
	vl_kas.data[4] = mag->servic.svc_end[1];
	vl_kas.data[5] = mag->servic.svc_end[2];

	vl_kas.sttim[0] = mag->servic.svc_tim[0];							// ��������
	vl_kas.sttim[1] = mag->servic.svc_tim[1];
	vl_kas.sttim[2] = mag->servic.svc_tim[2];
	vl_kas.sttim[3] = mag->servic.svc_tim[3];

	if( (99 == vl_kas.data[3]) && (99 == vl_kas.data[4]) && (99 == vl_kas.data[5]) ){ 	// ��������
		;
	}else{														// �������茔
		if( !datenearend( &vl_kas.data[3] ) ){					// �����؂�ԋ�����
			ret = 100;											// �ԋ�
		}
	}

	return( ret );
}


#if SYUSEI_PAYMENT
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �C�����Z�ް�����(�����v�Z�p�ɕϊ�����)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_scarchg( no )																   |*/
/*| PARAMETER	: no	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(NT4500EX�ް�)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	vl_scarchg( ushort no )
{

	if( ( no >= 1 )&&( no <= LOCK_MAX ) ){						// ���Ԉʒu�ԍ�����(1�`15?)

		no -= 1;												// �Ԏ�1(no=1)��sei_stp_dat[0]

		memset( &vl_tik, 0x00, sizeof( struct VL_TIK ) );		// ���Ԍ��ް�(vl_tik)������(0�ر)

		cr_dat_n = 0x26;										// ��ID���޾��(���Z���~��)
		vl_now = V_CHS;											// �Ǎ��݌���ʾ��(���Z���~��)

		vl_tik.syu = LockInfo[no].ryo_syu;						// ��ʾ��(���Ԉʒu�ݒ薈)

		card_use[USE_SVC] = (uchar)( syusei[no].sy_wmai +		// �g�p����(���~,�C���p)
						FLAPDT.flp_data[no].bk_wmai );
		vl_tik.wari = syusei[no].sy_wari +						// �������z(���~,�C���p)
						FLAPDT.flp_data[no].bk_wari;
		vl_tik.time = syusei[no].sy_time +						// �������Ԑ�(���~,�C���p)
						FLAPDT.flp_data[no].bk_time;
		vl_tik.pst = (uchar)FLAPDT.flp_data[no].bk_pst;			// ��������(���~,�C���p)

		vl_tik.chk = 0;											// ���Z�O�����

		vl_tik.cno = KIHON_PKNO;								// ���ԏꇂ���(��{���ԏꇂ)
		vl_tik.ckk = (short)CPrmSS[S_PAY][2];					// �����@No.
		vl_tik.hno = 0;											// ����No.
		vl_tik.mno = 0;											// �X�����

		vl_tik.tki = car_in_f.mon;								// ���Ԍ����
		vl_tik.hii = car_in_f.day;								// ���ԓ����
		vl_tik.jii = car_in_f.hour;								// ���Ԏ����
		vl_tik.fun = car_in_f.min;								// ���ԕ����

		syashu = vl_tik.syu;									// �Ԏ�

		hzuk.y = car_ot_f.year;
		hzuk.m = car_ot_f.mon;
		hzuk.d = car_ot_f.day;

		hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

		jikn.t = car_ot_f.hour;
		jikn.m = car_ot_f.min;

		jikn.s = 0;

		car_in.year  = car_in_f.year;							// ���ԔN
		car_in.mon   = vl_tik.tki;								//     ��
		car_in.day   = vl_tik.hii;								//     ��
		car_in.hour  = vl_tik.jii;								//     ��
		car_in.min   = vl_tik.fun;								//     ��
		car_in.week  = (char)youbiget( car_in.year ,			//     �j��
								(short)car_in.mon  ,
								(short)car_in.day  );
		car_ot.year = hzuk.y;									// �o�ɔN
		car_ot.mon  = hzuk.m;									//     ��
		car_ot.day  = hzuk.d;									//     ��
		car_ot.week = hzuk.w;									//     �j��
		car_ot.hour = jikn.t;									//     ��
		car_ot.min  = jikn.m;									//     ��
	}
}
#endif		// SYUSEI_PAYMENT

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �C���p���������(�����v�Z�p�ɕϊ�����)														   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_passet( no )																   |*/
/*| PARAMETER	: no	; ���������p���Ԉʒu�ԍ�(1�`324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	vl_passet( ushort no )
{
	short	yyy, mmm, ddd;
	char	c_prm;

	cr_dat_n = RID_APS;											// ID Code(1AH)
	memset( &tsn_tki, 0x00, sizeof( struct TSN_TKI) );

	tsn_tki.pkno = (short)((syusei[no-1].tei_syu&0xc0)>>6);		// ���ԏꇂ���
	tsn_tki.kind = (char)(syusei[no-1].tei_syu&0x3f);					// ��ʾ��
	tsn_tki.code = (short)syusei[no-1].tei_id;					// �l���޾��

	idnrmlzm( syusei[no-1].tei_sd, &yyy, &mmm, &ddd );			// �L�������J�n��
	tsn_tki.data[0] = (char)( yyy % 100 );
	tsn_tki.data[1] = (char)mmm;
	tsn_tki.data[2] = (char)ddd;

	idnrmlzm( syusei[no-1].tei_ed, &yyy, &mmm, &ddd );			// �L�������I����
	tsn_tki.data[3] = (char)( yyy % 100 );
	tsn_tki.data[4] = (char)mmm;
	tsn_tki.data[5] = (char)ddd;

	c_prm = (char)prm_get( COM_PRM,S_PAS,						// �������ݒ�Get
			(short)(5+10*(tsn_tki.kind-1)),1,2 );
	if( c_prm == 1 || c_prm == 2 ){								// ������or�J�n��������
		tsn_tki.data[0] = 90;									// 1990/1/1���
		tsn_tki.data[1] = 1;
		tsn_tki.data[2] = 1;
	}
	if( c_prm == 1 || c_prm == 3 ){								// ������or�I����������
		tsn_tki.data[3] = 79;									// 2079/12/31���
		tsn_tki.data[4] = 12;
		tsn_tki.data[5] = 31;
	}

	tsn_tki.status = 0;

	hzuk.y = CLK_REC.year;
	hzuk.m = CLK_REC.mont;
	hzuk.d = CLK_REC.date;
	hzuk.w = (char)youbiget( hzuk.y, (short)hzuk.m, (short)hzuk.d );

	jikn.t = CLK_REC.hour;
	jikn.m = CLK_REC.minu;
	jikn.s = 0;
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| �����؂�ԋ�����																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: datenearend()																	   |*/
/*| PARAMETER	: char *edate	; �N����														   |*/
/*| RETURN VALUE: ret	; 0=�ԋ߁i3���O�j�A0�ȊO=�ԋ߂łȂ�										   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: TF7700��藬�p																   |*/
/*| Date		: 2005-11-28																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static char	datenearend( char *edate )
{
	struct CAR_TIM	wk_e3;										// �����I��3���O
	char	date_e3[3];
	char	ret;
// MH322914 (s) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)
	short	i;
	uchar	uc;
// MH322914 (e) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)

	memset(&wk_e3,0,sizeof(wk_e3));
	if( edate[0] >= 80 )
		wk_e3.year = ((int)edate[0] + 1900);					// �����I���N
	else
		wk_e3.year = ((int)edate[0] + 2000);					// �����I���N

	wk_e3.mon  = edate[1];										// �����I�������
	wk_e3.day  = edate[2];										// �����I�������
	wk_e3.hour = 0;
	wk_e3.min  = 0;

// MH322914 (s) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)
//	ec62( &wk_e3 );												// 1���߂�
//	ec62( &wk_e3 );												// 2���߂�
//	ec62( &wk_e3 );												// 3���߂�
	ret = OK;
	switch (cr_dat_n) {
	case RID_APS:												// APS�����
		uc = (uchar)prm_get(COM_PRM, S_TIK, 13, 2, 1);			// 05-0013�D�E
		if (uc <= 15) {
			if (uc == 0) {
				uc = 3;											// ��̫�Ă�3���O
			}
			for (i = 0; i < uc; i++) {
				ec62( &wk_e3 );									// 1���߂�
			}
		} else {
			ret = NG;											// �����؂�\���Ȃ�
		}
		break;
// MH322914 (s) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)�񐔌������؂�\��
	case RID_KAI:												/* �񐔌�			*/
		uc = (uchar)prm_get(COM_PRM, S_PRP, 24, 2, 1);			/* 13-0024�D�E		*/
		if (uc <= 15) {											/*					*/
			if (uc == 0) {										/*					*/
				uc = 3;											/* ��̫�Ă�3���O	*/
			}													/*					*/
			for (i = 0; i < uc; i++) {							/*					*/
				ec62( &wk_e3 );									/* �P���߂�			*/
			}													/*					*/
		} else {												/*					*/
			ret = NG;											/* �����؂�\���Ȃ�	*/
		}														/*					*/
		break;													/*					*/
// MH322914 (e) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)�񐔌������؂�\��
	default:
		ec62( &wk_e3 );											//�@1���߂�
		ec62( &wk_e3 );											//�@2���߂�
		ec62( &wk_e3 );											//�@3���߂�
		break;
	}
// MH322914 (e) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)

	date_e3[0] = (char)(wk_e3.year % 100);
	date_e3[1] = wk_e3.mon;
	date_e3[2] = wk_e3.day;

// MH810101(S) R.Endo 2021/02/09 #5256 �y�݌v���̕]��NG�z����̗L�������؂�`�F�b�N�����Ȃ��ݒ�̏ꍇ�ɂ��A�܂��Ȃ��L�������؂��\�����Ă��܂��B
//	ret = ec081( date_e3 , edate );
	if ( ret == OK ) {
		ret = ec081( date_e3 , edate );
	}
// MH810101(E) R.Endo 2021/02/09 #5256 �y�݌v���̕]��NG�z����̗L�������؂�`�F�b�N�����Ȃ��ݒ�̏ꍇ�ɂ��A�܂��Ȃ��L�������؂��\�����Ă��܂��B

	return( ret );
}
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �ڍג��~�ر��������																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: vl_cyushiset( no )															   |*/
/*| PARAMETER	: no	; ���Ԉʒu�ԍ�(1�`9999)													   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: H.Sekiguchi																	   |*/
/*| Date		: 2006-10-15																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void vl_cyushiset( ulong no )
{
	struct CAR_TIM wk_time1,wk_time2;
	uchar	i;

	memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
	wrcnt_sub = 0;	//�󎚗p�J�E���^�[�ر
	
	for(i = 0 ; i < 10 ; i++){
		if(FLAPDT_SUB[i].WPlace == no){
			/*���Ɏ���*/
			wk_time1.year = car_in.year ;
			wk_time1.mon = car_in.mon ;
			wk_time1.day = car_in.day ;
			wk_time1.hour = car_in.hour ;
			wk_time1.min = car_in.min ;
			/*�ڍג��~�����ް�*/
			wk_time2.year = FLAPDT_SUB[i].TInTime.Year;
			wk_time2.mon = FLAPDT_SUB[i].TInTime.Mon;
			wk_time2.day = FLAPDT_SUB[i].TInTime.Day;
			wk_time2.hour = FLAPDT_SUB[i].TInTime.Hour;
			wk_time2.min = FLAPDT_SUB[i].TInTime.Min;
			
			if(0 == ec64(&wk_time1,&wk_time2)){
					Flap_Sub_Num = i;
					break;
			}
		}
	}
	if(i >= 10){
		Flap_Sub_Flg = 2;									//�Đ��Z&&�ڍג��~�ر��
	}else{
		Flap_Sub_Flg = 1;									//�Đ��Z&&�ڍג��~�ر�L
	}
}

// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)
//[]----------------------------------------------------------------------[]
///	@brief		���u���Z�f�[�^���Z�b�g(�����v�Z�p�ɃZ�b�g)
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2016/08/08<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2016 AMANO Corp.---[]
void vl_tikchg( void )
{
	cr_dat_n = RID_CKM;

	memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );

	vl_tik.tki = g_PipCtrl.stRemoteTime.InTime.Mon;			// ���Ԍ����
	vl_tik.hii = g_PipCtrl.stRemoteTime.InTime.Day;			// ���ԓ����
	vl_tik.jii = g_PipCtrl.stRemoteTime.InTime.Hour;		// ���Ԏ����
	vl_tik.fun = g_PipCtrl.stRemoteTime.InTime.Min;			// ���ԕ����
// MH810100(S) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	vl_tik.syu = LockInfo[OPECTL.Pr_LokNo-1].ryo_syu;
// MH810100(E) K.Onodera 2020/02/07 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	vl_tik.syu = g_PipCtrl.stRemoteTime.RyoSyu;				// �������

	syashu = vl_tik.syu;									// �Ԏ�

	car_in.mon   = vl_tik.tki;								//     ��
	car_in.day   = vl_tik.hii;								//     ��
	car_in.hour  = vl_tik.jii;								//     ��
	car_in.min   = vl_tik.fun;								//     ��
	car_in.year  = CLK_REC.year;
	if( (car_in.mon > CLK_REC.mont) ||						/* ���Ԍ��������݌����H */
		((car_in.mon == CLK_REC.mont) && (car_in.day > CLK_REC.date)) ){
		car_in.year--;
	}

	car_in.week  = (char)youbiget( car_in.year,				//     �j��
							(short)car_in.mon,
							(short)car_in.day );
}
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή��F���u���Z(���Ɏ����w��)

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//[]----------------------------------------------------------------------[]
///	@brief		�J�[�h��񂩂�A�����v�Z�p�f�[�^�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]----------------------------------------------------------------------[]
///	@date		Create	: 2019/11/15<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void vl_lcd_tikchg( void )
{
// GG129000(S) T.Nagai 2023/01/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�Ԕԃf�[�^ID�擾�����C���j
	uchar	i;
// GG129000(E) T.Nagai 2023/01/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�Ԕԃf�[�^ID�擾�����C���j

	cr_dat_n = RID_CKM;		// ���Ԍ��i���Z�O�j

	memset( &vl_tik, 0x00, sizeof( struct VL_TIK) );

	// ���ɔN��������
	             car_in.year = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.shYear;
	vl_tik.tki = car_in.mon  = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byMonth;
	vl_tik.hii = car_in.day  = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byDay;
	vl_tik.jii = car_in.hour = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byHours;
	vl_tik.fun = car_in.min  = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.dtEntryDateTime.dtTimeYtoSec.byMinute;

	// �j��
	car_in.week  = (char)youbiget( car_in.year, (short)car_in.mon, (short)car_in.day );

	// �������
	vl_tik.syu = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.shFeeType;
	syashu = vl_tik.syu;

	// �Ԕԏ��̃Z�b�g
	memset( &vl_car_no, 0, sizeof(vl_car_no) );

// GG129000(S) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/�R�����g�j
//	// ���(0=�ԔԌ���/1=��������)
	// ���(0=�ԔԌ���/1=��������/2=QR����)
// GG129000(E) H.Fujinaga 2022/12/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR�ǎ��ʑΉ�/�R�����g�j
	vl_car_no.CarSearchFlg = lcdbm_rsp_in_car_info_main.kind;

	// �ԔԌ����̏ꍇ��shaban[4];
// MH810100(S) S.Nishimoto 2020/04/07 �ÓI���(20200407:60)�Ή�
//	memcpy( vl_car_no.CarSearchData, &lcdbm_rsp_in_car_info_main.data.shabanSearch.shaban, sizeof(vl_car_no.CarSearchData) );
	memcpy( vl_car_no.CarSearchData, &lcdbm_rsp_in_car_info_main.data.shabanSearch.shaban, sizeof(lcdbm_rsp_in_car_info_main.data.shabanSearch.shaban) );
// MH810100(E) S.Nishimoto 2020/04/07 �ÓI���(20200407:60)�Ή�

	// �����⍇�����̎Ԃ̎Ԕ�
	memcpy( vl_car_no.CarNumber, lcdbm_rsp_in_car_info_main.shaban, sizeof(vl_car_no.CarNumber) );

	// �⍇���}�̔ԍ�
// GG129000(S) T.Nagai 2023/01/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�Ԕԃf�[�^ID�擾�����C���j
//	memcpy( vl_car_no.CarDataID, lcdbm_rsp_in_car_info_main.crd_info.byAskMediaNo, sizeof(vl_car_no.CarDataID) );
	for (i = 0; i < ONL_MAX_CARDNUM; i++) {
		// �}�X�^�[��񂩂�Ԕԃf�[�^ID�i���o��ID�j����������
		if (lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i].CardType == CARD_TYPE_INOUT_ID) {
			memcpy( vl_car_no.CarDataID,
					lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo.stCardDataInfo[i].byCardNo,
					sizeof(vl_car_no.CarDataID) );
			break;
		}
	}
// GG129000(E) T.Nagai 2023/01/22 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�Ԕԃf�[�^ID�擾�����C���j
}
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

