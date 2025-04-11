/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���ԕ��ޏW�v																				   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2005-02-01																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
// MH322914(S) K.Onodera 2017/01/05 AI-V�Ή�
#include	"flp_def.h"
// MH322914(E) K.Onodera 2017/01/05 AI-V�Ή�

static	char	getnumber( ulong );									/* �Y�����ԕ��އ��擾			*/
static	char	getnumber2( ushort );								/* �Y�����ԕ��އ��擾			*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���ԕ��ޏW�v���Z����																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kan_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N���p)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bunrui( ushort pos, ulong ryo ,char Syubetu)				/*								*/
{																	/*								*/
	char	bno;													/* �Y�����އ�					*/
	char	nno;													/* ���ޕ��@						*/
	char	bn_max;													/* ���ԕ��ގg�pMAX���ر			*/
	ulong	totim;													/* �����Ԏ���					*/
	ushort	id, it, od, ot;											/* ɰ�ײ�ޗp					*/
	SYUKEI	*ts;													/*								*/
	short	Num;													/* ���ʐݒ�ԍ�	(52 or 53)		*/
	char	Ichi;													/* ���ʐݒ�� �f�[�^�ʒu(1-6)	*/
	uchar	bn_syasyu;												/* ���ގԎ�i�[					*/
	uchar	b_typ = 0;
																	/*								*/
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
																	/*								*/
	if( 6 > Syubetu ) Num = 52;										/* �p�����[�^�ݒ�				*/
	else Num = 53;													/*								*/
																	/*								*/
	Ichi = (char)((Syubetu % 6) + 1);								/* �p�����[�^���ݒ�				*/
	bn_syasyu = (uchar)prm_get(COM_PRM, S_BUN, Num, 1, Ichi);		/* ���ގԎ�Z�b�g				*/
	if( (!bn_syasyu) && ( bn_syasyu >= 4 ) ) return;				/* �Ԏ�ݒ�Ȃ����P�`�R�ł͂Ȃ�	*/

	if( PayData.teiki.update_mon )	return;							/* �X�V���Z�͕��ޏW�v���Ȃ�		*/
																	/*								*/
	bn_max = (char)CPrmSS[S_BUN][2];								/* ���ސ�get					*/
	if( !bn_max )	return;											/* ���ސ��ݒ�Ȃ�				*/
																	/*								*/
	nno = (char)CPrmSS[S_BUN][1];									/*								*/
	switch( nno )													/* ���ԕ��ޏW�v���@�ɂ�蕪��	*/
	{																/*								*/
	case 1:															/* ���Ԏ��ԕʑ䐔�W�v			*/
	case 2:															/* ���Ԏ��ԕʑ䐔�E�����W�v		*/
// �s��C��(S) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
		// ���Ɏ����Ȃ��H
		if( (!car_in_f.year && !car_in_f.mon && !car_in_f.day &&
			!car_in_f.hour && !car_in_f.min) ||
			( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode <= FLAP_CTRL_MODE3 ) ){
			bno = 0;
			b_typ = 3;
			ts->Bunrui1_cnt2[bn_syasyu-1]++;
			if( nno == 2 ){
				ts->Bunrui1_ryo2[bn_syasyu-1] += ryo;
			}
		}
		else{
// �s��C��(E) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
		id = dnrmlzm( car_in_f.year,								/* ���ԓ�ɰ�ײ��				*/
				(short)car_in_f.mon, (short)car_in_f.day );			/*								*/
		it = (ushort)tnrmlz( 0, 0, (short)car_in_f.hour,			/* ���Ԏ���ɰ�ײ��				*/
							(short)car_in_f.min );					/*								*/
		od = dnrmlzm( car_ot_f.year,								/* �o�ɓ�ɰ�ײ��				*/
				(short)car_ot_f.mon, (short)car_ot_f.day );			/*								*/
		ot = (ushort)tnrmlz( 0, 0, (short)car_ot_f.hour,			/* �o�Ɏ���ɰ�ײ��				*/
							(short)car_ot_f.min );					/*								*/
																	/*								*/
		if( id == od ){												/* ���ԓ����o�ԓ�?				*/
			totim = (ulong)(ot - it);								/* �����Ԏ��ԎZ�o				*/
		}else{														/*								*/
			if(( od - id ) > 70 ){									/* 70���ȏ�(99990����)����?		*/
				totim = 100000L;									/* �����Ԏ���100000����			*/
			}else{													/*								*/
				totim = ((ulong)(od - id - 1) * 1440L)				/* �����Ԏ��ԎZ�o				*/
						+ (ulong)ot									/*								*/
						+ (ulong)(1440 - it);						/*								*/
			}														/*								*/
		}															/*								*/
																	/*								*/
		bno = getnumber( totim );									/* �Y�����ԕ��އ��擾			*/
		if(( bno != 0 )&&( bno <= bn_max )){						/* ���އ����L�ŁA�g�pMAX�ȉ��Ȃ�*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* �Y�����ނ̶��ı���			*/
			if( nno == 2 ){											/* ���Ԏ��ԕʑ䐔�E�����W�v?	*/
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1] += ryo;			/* �������Z						*/
			}														/*								*/
			b_typ = 1;
		}else{														/* ���އ�����					*/
			ts->Bunrui1_cnt1[bn_syasyu-1]++;						/* �敪�ȏ㒓�ԕ��ޑ䐔���ı���	*/
			if( nno == 2 ){											/* ���Ԏ��ԕʑ䐔�E�����W�v?	*/
				ts->Bunrui1_ryo1[bn_syasyu-1] += ryo;				/* �敪�ȏ㒓�ԕ��ޗ������Z		*/
			}														/*								*/
			b_typ = 2;
		}															/*								*/
// �s��C��(S) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
		}
// �s��C��(E) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 3:															/* ���ԗ����ʑ䐔�W�v			*/
	case 4:															/* ���ԗ����ʑ䐔�E�����W�v		*/
		bno = getnumber( ryo );										/* �Y�����ԕ��އ��擾			*/
		if(( bno != 0 )&&( bno <= bn_max )){						/* ���އ����L�ŁA�g�pMAX�ȉ��Ȃ�*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* �Y�����ނ̶��ı���			*/
			if( nno == 4 ){											/* ���ԗ����ʑ䐔�E�����W�v?	*/
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1] += ryo;				/* �������Z						*/
			}														/*								*/
			b_typ = 1;
		}else{														/* ���އ�����					*/
			ts->Bunrui1_cnt1[bn_syasyu-1]++;						/* �敪�ȏ㒓�ԕ��ޑ䐔���ı���	*/
			if( nno == 4 ){											/* ���ԗ����ʑ䐔�E�����W�v?	*/
				ts->Bunrui1_ryo1[bn_syasyu-1] += ryo;				/* �敪�ȏ㒓�ԕ��ޗ������Z		*/
			}														/*								*/
			b_typ = 2;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 5:															/* ���Ԏ����ʐ��Z�䐔�W�v		*/
		it = (ushort)tnrmlz( 0, 0, (short)car_in_f.hour,			/* ���Ԏ���ɰ�ײ��				*/
							(short)car_in_f.min );					/*								*/
// �s��C��(S) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
//		bno = getnumber2( it );										/* �Y�����ԕ��އ��擾			*/
		if( (!car_in_f.year && !car_in_f.mon && !car_in_f.day &&
			!car_in_f.hour && !car_in_f.min) ||
			( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode <= FLAP_CTRL_MODE3 ) ){
			bno = (char)NG;
		}else{
			bno = getnumber2( it );									// �Y�����ԕ��އ��擾
		}
// �s��C��(E) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
		if( bno == (char)NG ){										/* �Y�����ԕ��އ����擾�H		*/
			ts->Bunrui1_cnt2[bn_syasyu-1]++;						/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* �Y�����ނ̶��ı���			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 6:															/* ���Z�����ʐ��Z�䐔�W�v		*/
		ot = (ushort)tnrmlz( 0, 0, (short)car_ot_f.hour,			/* �o�Ɏ���ɰ�ײ��				*/
							(short)car_ot_f.min );					/*								*/
		bno = getnumber2( ot );										/* �Y�����ԕ��އ��擾			*/
		if( bno == (char)NG ){										/* �Y�����ԕ��އ����擾�H		*/
			ts->Bunrui1_cnt2[bn_syasyu-1]++;						/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* �Y�����ނ̶��ı���			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
		break;														/*								*/
	case 7:															/* ���ԁE���Z�����ʐ��Z�䐔�W�v	*/
		it = (ushort)tnrmlz( 0, 0, (short)car_in_f.hour,			/* ���Ԏ���ɰ�ײ��				*/
							(short)car_in_f.min );					/*								*/
// �s��C��(S) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
//		bno = getnumber2( it );										/* �Y�����ԕ��އ��擾			*/
		if( (!car_in_f.year && !car_in_f.mon && !car_in_f.day &&
			!car_in_f.hour && !car_in_f.min) ||
			( OPECTL.CalStartTrigger == CAL_TRIGGER_REMOTECALC_FEE && FLAPDT.flp_data[OPECTL.Pr_LokNo-1].mode <= FLAP_CTRL_MODE3 ) ){
			bno = (char)NG;
		}else{
			bno = getnumber2( it );									// �Y�����ԕ��އ��擾
		}
// �s��C��(E) K.Onodera 2016/11/18 #1570 ���ޏW�v�����Ԏ��ԕs���ɕ��ނ���Ȃ�
		if( bno == (char)NG ){										/* �Y�����ԕ��އ����擾�H		*/
			ts->Bunrui1_cnt2[bn_syasyu-1]++;						/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_cnt[bn_syasyu-1][bno-1]++;					/* �Y�����ނ̶��ı���			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu1 = bno*100;
			syusei[pos-1].bun_syu1 += bn_syasyu*10;
			syusei[pos-1].bun_syu1 += b_typ;
		}
																	/*								*/
		ot = (ushort)tnrmlz( 0, 0, (short)car_ot_f.hour,			/* �o�Ɏ���ɰ�ײ��				*/
							(short)car_ot_f.min );					/*								*/
		bno = getnumber2( ot );										/* �Y�����ԕ��އ��擾			*/
		if( bno == (char)NG ){										/* �Y�����ԕ��އ����擾�H		*/
			ts->Bunrui1_ryo2[bn_syasyu-1]++;						/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			b_typ = 3;
		}else{														/*								*/
			ts->Bunrui1_ryo[bn_syasyu-1][bno-1]++;					/* �Y�����ނ̶��ı���			*/
			b_typ = 1;
		}															/*								*/
		if( pos ){
			syusei[pos-1].bun_syu2 = bno*100;
			syusei[pos-1].bun_syu2 += bn_syasyu*10;
			syusei[pos-1].bun_syu2 += b_typ;
		}
		break;														/*								*/
	}																/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �Y�����ԕ��އ��擾����																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: getnumber( data )																   |*/
/*| PARAMETER	: data	: ���ނ��闿��(�~)���͎���(��)											   |*/
/*| RETURN VALUE: ans	: 0:�Y�����Ȃ�, 1-48:�Y����												   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N���p)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	char	getnumber( ulong data )								/*								*/
{																	/*								*/
	short	cnt, num;												/*								*/
	char	ans;													/*								*/
																	/*								*/
	num = (short)CPrmSS[S_BUN][2];									/* ���ԕ��ސ�					*/
																	/*								*/
	ans = 0;														/*								*/
	for( cnt = 0; cnt < num; cnt++ ){								/* �Y�����ԕ��އ��̎擾			*/
		if( data <= (ulong)(CPrmSS[S_BUN][3+cnt]*10L) ){			/*								*/
			ans = (char)(cnt+1);									/*								*/
			break;													/*								*/
		}															/*								*/
	}																/*								*/
	return( ans );													/*								*/
}																	/*								*/
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| �Y�����ԕ��އ��擾����2																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: getnumber2( data )															   |*/
/*| PARAMETER	: data	: ���ނ��鎞��(��)														   |*/
/*| RETURN VALUE: ans	: 0:�Y�����Ȃ�, 1-48:�Y����												   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N���p)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
static	char	getnumber2( ushort indat )							/*								*/
{																	/*								*/
	short   bundat[48+1];											/* ���ԕ��ޏW�v�ݒ�l�@�@�@�@�@ */
	char	ret = NG;												/* ���^���R�[�h�@�@�@�@�@�@�@�@ */
	char	bn_max;													/* ���ԕ��ގg�p�l�`�w���G���A�@ */
	char	i;														/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
																	/*								*/
	bn_max = (char)CPrmSS[S_BUN][2];								/* ���ԕ��ސ�					*/
																	/*								*/
	bundat[0] = (ushort)prm_tim( 0, S_BUN, (short)(3+bn_max-1) );	/* �ݒ�f�[�^					*/
	for( i = 0; i < bn_max; i ++ )									/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
	{																/*								*/
		bundat[i+1] = (ushort)prm_tim( 0, S_BUN, (short)(3+i) );	/* �ݒ�f�[�^					*/
	}																/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
																	/*								*/
	for( i = 0; i < bn_max; i ++ )									/* ���ނ�I�ʂ���B�@�@�@�@�@�@ */
	{																/*								*/
		if( bundat[i] < bundat[i+1] ){								/* ���̕��ސݒ�z���ް��̂ق����傫�� */
			if(( indat > bundat[i] )&&( indat <= bundat[i+1] )){	/* ���ސݒ�l�͈̔͂ɓ������@�@ */
				ret = (char)(i+1);									/* �Y���m���D���Z�b�g�@�@�@�@�@ */
				break;												/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
			}														/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
		}else if( bundat[i] > bundat[i+1] ){						/* 0:00���܂������ݒ�̎��@�@�@ */
			if( indat > bundat[i] ){								/* ���ސݒ�l���f�[�^���傫���ꍇ */
				ret = (char)(i+1);									/* �Y���m���D���Z�b�g�@�@�@�@�@ */
				break;												/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
			}else if( indat <= bundat[i+1] ){						/* ���̕��ސݒ�l���f�[�^���������ꍇ */
				ret = (char)(i+1);									/* �Y���m���D���Z�b�g�@�@�@�@�@ */
				break;												/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
			}														/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
		}															/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
	}																/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
	return( ret );													/*								*/
}																	/*								*/

/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���ԕ��ޏW�v�i�C�����Z�j����																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: bunrui_syusei( ushort pos, ulong ryo )										   |*/
/*| PARAMETER	: P1	: �Ԏ��ʒu																   |*/
/*|				: P2	: ����																	   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N���p)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bunrui_syusei( ushort pos, ulong ryo )						/*								*/
{																	/*								*/
	uchar	bno;													/* �Y�����އ�					*/
	uchar	nno;													/* ���ޕ��@						*/
	SYUKEI	*ts;													/*								*/
	uchar	bn_syasyu;												/* ���ގԎ�i�[					*/
	uchar	b_typ;
																	/*								*/

	if( !syusei[pos-1].bun_syu1 )	return;

	bn_syasyu = (uchar)((syusei[pos-1].bun_syu1/10)%10);			/* �������Z						*/
	bno = (uchar)(syusei[pos-1].bun_syu1/100);
	b_typ = (uchar)(syusei[pos-1].bun_syu1%10);
																	/*								*/
	ts = &sky.tsyuk;												/*								*/
																	/*								*/
	nno = (char)CPrmSS[S_BUN][1];									/*								*/
	switch( nno )													/* ���ԕ��ޏW�v���@�ɂ�蕪��	*/
	{																/*								*/
	case 1:															/* ���Ԏ��ԕʑ䐔�W�v			*/
	case 2:															/* ���Ԏ��ԕʑ䐔�E�����W�v		*/
		switch(b_typ)
		{
		case 1:
			if( (ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0)&&
				(ts->Bunrui1_ryo[bn_syasyu-1][bno-1] >= ryo) ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* �Y�����ނ̶��ı���			*/
				if( nno == 2 ){										/* ���Ԏ��ԕʑ䐔�E�����W�v?	*/
					ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= ryo;		/* �������Z						*/
				}													/*								*/
			}
			break;
		case 2:
			if( (ts->Bunrui1_cnt1[bn_syasyu-1] > 0)&&
				(ts->Bunrui1_ryo1[bn_syasyu-1] >= ryo) ){
				ts->Bunrui1_cnt1[bn_syasyu-1]--;					/* �敪�ȏ㒓�ԕ��ޑ䐔���ı���	*/
				if( nno == 2 ){										/* ���Ԏ��ԕʑ䐔�E�����W�v?	*/
					ts->Bunrui1_ryo1[bn_syasyu-1] -= ryo;			/* �敪�ȏ㒓�ԕ��ޗ������Z		*/
				}													/*								*/
			}
			break;
		}
		break;
	case 3:															/* ���ԗ����ʑ䐔�W�v			*/
	case 4:															/* ���ԗ����ʑ䐔�E�����W�v		*/
		switch(b_typ)
		{
		case 1:
			if( (ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0)&&
				(ts->Bunrui1_ryo[bn_syasyu-1][bno-1] >= ryo) ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* �Y�����ނ̶��ı���			*/
				if( nno == 4 ){										/* ���ԗ����ʑ䐔�E�����W�v?	*/
					ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= ryo;		/* �������Z						*/
				}													/*								*/
			}
			break;
		case 2:
			if( (ts->Bunrui1_cnt1[bn_syasyu-1] > 0)&&
				(ts->Bunrui1_ryo1[bn_syasyu-1] >=ryo) ){
				ts->Bunrui1_cnt1[bn_syasyu-1]++;					/* �敪�ȏ㒓�ԕ��ޑ䐔���ı���	*/
				if( nno == 4 ){										/* ���ԗ����ʑ䐔�E�����W�v?	*/
					ts->Bunrui1_ryo1[bn_syasyu-1] -= ryo;			/* �敪�ȏ㒓�ԕ��ޗ������Z		*/
				}													/*								*/
			}
			break;
		}
		break;
	case 5:															/* ���Ԏ����ʐ��Z�䐔�W�v		*/
		if( b_typ == 3 ){											/* �Y�����ԕ��އ����擾�H		*/
			if( ts->Bunrui1_cnt2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_cnt2[bn_syasyu-1]--;					/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			}
		}else if( b_typ == 1 ){										/*								*/
			if( ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* �Y�����ނ̶��ı���			*/
			}
		}															/*								*/
		break;														/*								*/
	case 6:															/* ���Z�����ʐ��Z�䐔�W�v		*/
		if( b_typ == 3 ){											/* �Y�����ԕ��އ����擾�H		*/
			if( ts->Bunrui1_cnt2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_cnt2[bn_syasyu-1]--;					/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			}
		}else if( b_typ == 1 ){										/*								*/
			if( ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* �Y�����ނ̶��ı���			*/
			}
		}															/*								*/
		break;														/*								*/
	case 7:															/* ���ԁE���Z�����ʐ��Z�䐔�W�v	*/
		if( b_typ == 3 ){											/* �Y�����ԕ��އ����擾�H		*/
			if( ts->Bunrui1_cnt2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_cnt2[bn_syasyu-1]--;					/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			}
		}else if( b_typ == 1 ){										/*								*/
			if( ts->Bunrui1_cnt[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_cnt[bn_syasyu-1][bno-1]--;				/* �Y�����ނ̶��ı���			*/
			}
		}															/*								*/

		bn_syasyu = (uchar)((syusei[pos-1].bun_syu2/10)%10);		/* �������Z						*/
		bno = (uchar)(syusei[pos-1].bun_syu2/100);
		b_typ = (uchar)(syusei[pos-1].bun_syu2%10);

		if( b_typ == 3 ){											/* �Y�����ԕ��އ����擾�H		*/
			if( ts->Bunrui1_ryo2[bn_syasyu-1] > 0 ){
				ts->Bunrui1_ryo2[bn_syasyu-1]--;					/* �敪�s�����ԕ��ޑ䐔���ı���	*/
			}
		}else{														/*								*/
			if( ts->Bunrui1_ryo[bn_syasyu-1][bno-1] > 0 ){
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1]--;				/* �Y�����ނ̶��ı���			*/
			}
		}															/*								*/
		break;														/*								*/
	}																/*								*/
}																	/*								*/

// �d�l�ύX(S) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����
/*[]----------------------------------------------------------------------------------------------[]*/
/*| ���ԕ��ޏW�v���Z����																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: kan_syuu( void )																   |*/
/*| PARAMETER	: void	: 																		   |*/
/*| RETURN VALUE: void	: 																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(TF4800N���p)															   |*/
/*| Date		: 2005-02-01																	   |*/
/*| Update		:																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	bunrui_Erace( Receipt_data* pay, ulong back, char syu )
{
	char	bno;					// �Y�����އ�
	char	bn_max;					// ���ԕ��ގg�pMAX���ر
	ulong	totim;					// �����Ԏ���
	ushort	id, it, od, ot;			// ɰ�ײ�ޗp
	ushort	i;
	SYUKEI	*ts;
	uchar	bn_syasyu;				// ���ގԎ�i�[
	ulong	ryo = 0;

	ts = &sky.tsyuk;

	// ���ގԎ�Z�b�g
	if( 6 > pay->syu ){
		bn_syasyu = (uchar)prm_get( COM_PRM, S_BUN, 52, 1, (char)( (pay->syu % 6) + 1 ) );
	}else{
		bn_syasyu = (uchar)prm_get( COM_PRM, S_BUN, 53, 1, (char)( (pay->syu % 6) + 1 ) );
	}
	// �Ԏ�ݒ�Ȃ����P�`�R�ł͂Ȃ�
	if( (!bn_syasyu) && ( bn_syasyu >= 4 ) ){
		return;
	}

	bn_max = (char)CPrmSS[S_BUN][2];
	// ���ސ��ݒ�Ȃ��H
	if( !bn_max ){
		return;
	}

	//���ԕ��ޏW�v���@�ɂ�蕪��
	switch( CPrmSS[S_BUN][1] )
	{
		case 2:			// ���Ԏ��ԕʑ䐔�E�����W�v
			// ���Ɏ����Ȃ��H
			if( !pay->TInTime.Year && !pay->TInTime.Mon && !pay->TInTime.Day &&
				!pay->TInTime.Hour && !pay->TInTime.Min ){
				ts->Bunrui1_ryo2[bn_syasyu-1] -= back;
			}
			else{
				id = dnrmlzm( pay->TInTime.Year, (short)pay->TInTime.Mon, (short)pay->TInTime.Day );		// ���ɓ��m�[�}���C�Y
				it = (ushort)tnrmlz( 0, 0, (short)pay->TInTime.Hour, (short)pay->TInTime.Min );				// ���Ɏ����m�[�}���C�Y
				od = dnrmlzm( pay->TOutTime.Year, (short)pay->TOutTime.Mon, (short)pay->TOutTime.Day );		// �o�ɓ��m�[�}���C�Y
				ot = (ushort)tnrmlz( 0, 0, (short)pay->TOutTime.Hour, (short)pay->TOutTime.Min );			// �o�Ɏ����m�[�}���C�Y
				// ���ɓ����o�ɓ��H
				if( id == od ){
					totim = (ulong)(ot - it);		// �����Ԏ��ԎZ�o
				}else{
					// 70���ȏ�(99990����)����?
					if(( od - id ) > 70 ){
						totim = 100000L;			// �����Ԏ���100000����
					}else{
						totim = ((ulong)(od - id - 1) * 1440L) + (ulong)ot + (ulong)(1440 - it);
					}
				}

				bno = getnumber( totim );							// �Y�����ԕ��އ��擾
				// ���އ����L�ŁA�g�pMAX�ȉ��H
				if(( bno != 0 )&&( bno <= bn_max )){
					ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= back;
				}
				// ���އ������H
				else{
					ts->Bunrui1_ryo1[bn_syasyu-1] -= back;
				}
			}
			break;

		case 4:			// ���ԗ����ʑ䐔�E�����W�v
		{
			ulong	wari_ryo = 0;
			ulong	ppc_ryo = 0;
			wari_tiket	wari_dt;
			for( i=0; i<WTIK_USEMAX; i++ ){
				disc_wari_conv( &pay->DiscountData[i], &wari_dt );
				switch( wari_dt.tik_syu ){
					case SERVICE:	/* ���޽��						*/
					case KAKEURI:	/* �|����						*/
						// �U�֌���� = �U�֐��ʖ��́A�����I�ɓK�p
						if( pay->syu == syu || prm_get(COM_PRM, S_CEN, 32, 1, 1) == 1 ){
							wari_ryo += wari_dt.ryokin;
						}
						break;
					case KAISUU:	/* �񐔌�						*/
					case MISHUU:	/* ������						*/
					case FURIKAE:	/* �U�֊z						*/
					case SYUUSEI:	/* �C���z						*/
						wari_ryo += wari_dt.ryokin;
						break;
					case	PREPAID:										/* �v���y�C�h�J�[�h				*/
						ppc_ryo += wari_dt.ryokin;							/* ����߲�޶��ގg�p�������Z		*/
						break;												/*								*/
				}															/*								*/
			}
			// ���ԗ��������߂�i�������܂ށ^�܂܂Ȃ��ݒ�ɏ]���j
			switch( prm_get(COM_PRM, S_TOT, 12, 1, 2) ){					// ���ޏW�v�̒��ԗ��������i�ݒ�j�ɂ�蕪��
				// ������̗����𒓎ԗ����Ƃ���i�v���y�͌����̎��z�Ƃ���j
				case 1:
					if( pay->WPrice >= (wari_ryo + pay->credit.pay_ryo) )
						ryo = (pay->WPrice) - wari_ryo - pay->credit.pay_ryo;	// ���ԗ����|��������
					break;
				// ������̗����𒓎ԗ����Ƃ���i�v���y�͊����z�Ƃ���j
				case 2:
					if( pay->WPrice >= (wari_ryo + ppc_ryo + ryo_buf.credit.pay_ryo + pay->Electron_data.Suica.pay_ryo) )
						ryo = (pay->WPrice) - wari_ryo - ppc_ryo - pay->credit.pay_ryo - pay->Electron_data.Suica.pay_ryo;	// ���ԗ����|���������|PPC�����|�d�q�Ȱ����
					break;
			}
			bno = getnumber( ryo );
			// ���އ����L�ŁA�g�pMAX�ȉ��H
			if(( bno != 0 )&&( bno <= bn_max )){
				ts->Bunrui1_ryo[bn_syasyu-1][bno-1] -= back;
			}
			// �敪�ȏ㒓�ԕ��ޗ����H
			else{
				ts->Bunrui1_ryo1[bn_syasyu-1] -= back;
			}
		}
		break;

		default:
			break;
	}
}
// �d�l�ύX(E) K.Onodera 2016/12/13 #1674 �W�v�̐��Z�����ɑ΂���U�֕��̉��Z���@��ύX����