/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		IF�ՒʐM-LOCKҲݲ���̪��																			   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	lkcomApi.c																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �EIF�ՂƂ̒ʐM�y��MAIN����Ƃ̲���̪��																	   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	<string.h>															/*								*/
#include	<stdlib.h>															/*								*/
#include	"iodefine.h"														/*								*/
#include	"system.h"															/*								*/
#include	"flp_def.h"															/*								*/
#include	"prm_tbl.h"															/*								*/
#include	"LKcom.h"															/*								*/
#include	"LKmain.h"															/*								*/
#include	"mem_def.h"															/*								*/
#include	"rkn_def.h"															/*								*/
#include	"rkn_cal.h"															/*								*/
#include	"ope_def.h"															/*								*/
#include	"common.h"															/*								*/
#include	"ntnet.h"															/*								*/
#include	"message.h"
#include	"mnt_def.h"															/*								*/
#include	"IFM.h"

// ����ُ�Ԓ�`
enum {
	IBCCOM_IBCSND_NORMAL,			// ����
	IBCCOM_IBCSND_ERROR,			// �ُ�
	IBCCOM_IBCSND_ERRRSLT,			// IBC�O���ް�������
};

// IBC����ّ��M���
enum {
	IBCCOM_IBCSND_IDLE,				// �����
	IBCCOM_IBCSND_WAIT_SEND,		// ���M�����҂�
};

#define	WAIT_RESPONSE_TIME		(60000+1) / 500		// ���M���ʉ����҂�����(60s)
#define	TIMEOUT_RETRY_MAX		3					// ���M���ʉ�����M��ѱ�Ăɂ�������ײ��
#define	TIMEOUT_RETRY_MAX_RSLT	1					// ���M�����ް���IBC�O���ް��������ǉ��������Ƃ�����ײ��
#define	BLOCK_RETRY_MAX			3					// ��ۯ�No.�ُ펞�ɂ�������ײ��
#define	SERIAL_NO_MAX			99					// �ǂ��ԍő吔
#define	QUE_NUM_MAX				10					// �e����ւ̍ő�į���

/* ��M�f�[�^�敪 */
#define DV_FLPCTRL_I	0x05	/* �ׯ�ߏ�� */
#define DV_FLPCTRL_A	0x06	/* �S�ׯ�ߏ�� */
#define DV_FLPTST		0x07	/* ýėv�� */
#define DV_FLPMNT		0x08	/* ����ݽ���v�� */

uchar	DownLockFlag[LOCK_MAX];


// function
char		LKcom_SetDtPkt( uchar, uchar, ushort, uchar );						/*								*/
uchar		LKcom_RcvDataGet( ushort *cnt );									/*								*/
void		LKcom_SetLockMaker( void );											/*								*/
void		LKopeApiLKCtrl( ushort, uchar );									/* ۯ����u�w��					*/
void		LKopeApiLKCtrl_All(uchar, uchar);
uchar		get_lktype(uchar lok_syu);
short		LKopeGetLockNum( uchar, ushort, ushort * );							/*								*/

#if (1 == AUTO_PAYMENT_PROGRAM)
void LK_AutoPayment_Send(uchar index, uchar tno, uchar lkno, uchar kind);
#endif
																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�ް��߹�đ��M�v���iAPI�j																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKcom_SetDtPkt(par1,par2,par3)															   |*/
/*| PARAMETER	: par1	: �ް�ID.																			   |*/
/*|				: par2	: �[����																			   |*/
/*|				: par3	: ۯ����u��																			   |*/
/*|				: par4	: �����敪(�����ް��ɕK�v�ƂȂ鏈���敪)											   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�E���ع���݂ɂđ��M�������ް����ޯ̧�־�Ă���															   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
char	LKcom_SetDtPkt(															/*								*/
uchar	did,																	/* �ް�ID						*/
uchar	tno,																	/* �[��No.(-1�Ƃ���)			*/
ushort	lkno,																	/* ۯ����u��(1�`324)			*/
uchar	kind )																	/* �����敪(�����ް��̂ݗL��)	*/
{																				/*								*/
ushort			i;																/*								*/
t_LKcomCtrl		*c_dp;															/*								*/
ushort			wk_l;															/* ܰ��Fۯ����u��0�`324-1		*/
	wk_l = lkno - 1;															/*								*/

	if( IFS_CRR_OFFSET > tno ){													/* CRR��ւ̃A�N�Z�X�ł͂Ȃ�	*/
		if( LKcom_Search_Ifno( tno ) ){											/* ����ٔԍ�����CRR��CRA���𔻒f */
			if(!( GetCarInfoParam() & 0x04 )){									/* CRR��ڑ���������Δ�����	*/
				return(0);														/* 								*/
			}
		}else{
			if(!( GetCarInfoParam() & 0x01 )){									/* CRA��ڑ���������Δ�����	*/
				return(0);														/* 								*/
			}
		}
	}
	switch( did ){																/*								*/
																				/*								*/
																				/*								*/
	case LK_SND_CTRL_ID:														/* ۯ����u�����ް�				*/
		if( LOCK_CTRL_BUFSIZE_MAX <= LKcomTerm.CtrlInfo.Count ){				/* �ޯ̧FULL���				*/
			++LKcomTerm.CtrlInfo.R_Index;										/* �Â��ް�������				*/
			if( LOCK_CTRL_BUFSIZE_MAX <= LKcomTerm.CtrlInfo.R_Index ){			/* 								*/
				LKcomTerm.CtrlInfo.R_Index = 0;									/*								*/
			}																	/* 								*/
			--LKcomTerm.CtrlInfo.Count;											/* 								*/
		}																		/*								*/
																				/*								*/
		c_dp = &LKcomTerm.CtrlData[LKcomTerm.CtrlInfo.W_Index];					/*								*/
																				/*								*/
		c_dp->did4 = LK_SND_CTRL_ID;											/* �ް�ID						*/
		c_dp->kind = kind;														/* �����敪						*/
		c_dp->tno = tno;														// CRB�C���^�[�t�F�[�X�̃^�[�~�i��No
		if( wk_l < LOCK_MAX ){													/* �Ԏ�0�`324-1?				*/
			if(LKcom_Search_Ifno(tno) == 0) {									// ���b�N�̏ꍇ
				c_dp->lock = LockInfo[wk_l].lok_no;								/* ۯ����uNo.					*/
			} else {															// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
				c_dp->lock = 1;													/* ۯ����uNo.					*/
			}
			if(wk_l < FLAP_NUM_MAX)												/*  ���u��ʂ��t���b�v			*/
			{
				c_dp->clos_tm = 0;												/* ����M���o�͎���			*/
				c_dp->open_tm = 0;												/* �J����M���o�͎���			*/
				if(kind == 1)													/* ����v�����㏸ 				*/
				{
					/* �t���b�v�㏸�J�n���o�^ */
					IoLog_write(IOLOG_EVNT_FLAP_UP_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
				else if(kind == 2)												/* ����v�������~ 				*/
				{
					/* �t���b�v���~�J�n���o�^ */
					IoLog_write(IOLOG_EVNT_FLAP_DW_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
			}
			else																/* ���u��ʂ����b�N				*/
			{
				c_dp->clos_tm = LockMaker[ LockInfo[wk_l].lok_syu - 1 ].clse_tm;	/* ����M���o�͎���			*/
				c_dp->open_tm = LockMaker[ LockInfo[wk_l].lok_syu - 1 ].open_tm;	/* �J����M���o�͎���			*/
				if(kind == 1)													/* ����v�����㏸ 				*/
				{
					/* ���փ��b�N�J�n���o�^ */
					IoLog_write(IOLOG_EVNT_LOCK_CL_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
				else if(kind == 2)												/* ����v�������~ 				*/
				{
					/* ���փ��b�N�J�J�n���o�^ */
					IoLog_write(IOLOG_EVNT_LOCK_OP_STA, (ushort)LockInfo[wk_l].posi, 0, 0);
				}
			}
		}else{																	/*								*/
			c_dp->lock = 0;														/* ۯ����uNo.					*/
			c_dp->clos_tm = 0;													/* ����M���o�͎���			*/
			c_dp->open_tm = 0;													/* �J����M���o�͎���			*/
			for(i = INT_CAR_START_INDEX; i < LOCK_MAX; i++)						/* �Ԏ����Ǘ��e�[�u������ 	*/
			{
				WACDOG;															// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
				if(LockInfo[i].if_oya == tno)									/* IF��Ղ���v 				*/
				{
					if(i < FLAP_NUM_MAX)										/* ���u��ʂ��t���b�v			*/
					{
						if(kind == 5)											/* ����v�����㏸ 				*/
						{
							/* �t���b�v�㏸�J�n���o�^ */
							IoLog_write(IOLOG_EVNT_FLAP_UP_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
						else if(kind == 6)										/* ����v�������~ 				*/
						{
							/* �t���b�v���~�J�n���o�^ */
							IoLog_write(IOLOG_EVNT_FLAP_DW_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
					}
					else														/* ���u��ʂ����b�N				*/
					{
						if(kind == 5)											/* ����v�����㏸ 				*/
						{
							/* ���փ��b�N�J�n���o�^ */
							IoLog_write(IOLOG_EVNT_LOCK_CL_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
						else if(kind == 6)										/* ����v�������~ 				*/
						{
							/* ���փ��b�N�J�J�n���o�^ */
							IoLog_write(IOLOG_EVNT_LOCK_OP_STA, (ushort)LockInfo[i].posi, 0, 0);
						}
					}
				}
			}
		}																		/*								*/
																				/*								*/
		++LKcomTerm.CtrlInfo.W_Index;											/*								*/
		if( LOCK_CTRL_BUFSIZE_MAX <= LKcomTerm.CtrlInfo.W_Index ){				/*								*/
			LKcomTerm.CtrlInfo.W_Index = 0;										/*								*/
		}																		/*								*/
		++LKcomTerm.CtrlInfo.Count;												/*								*/
		break;																	/*								*/
																				/*								*/
	default:																	/*								*/
		break;																	/*								*/
	}																			/*								*/
	return( 0 );																/*								*/
}																				/*								*/
																				/*								*/
/*	���ع���ݑ��Ŏg�p����֐�						 	*/
/*	��M�����ް������ް�̫�ϯĂɉ������ް����擾����	*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	��M�ް��擾(API)																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKcom_RcvDataGet()																		   |*/
/*| PARAMETER	: par1	: �i�[��ۯ���																		   |*/
/*|				: par2	: �i�[����	0:������ް��̂ݱ����ޯ̧�֊i�[											   |*/
/*|									1:������ް����ꎞ�ޯ̧�֊i�[											   |*/
/*|									2:����ƈꎞ�ޯ̧���ް�������ޯ̧�֊i�[								   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �E���ع���݂ֈ����n���p�ޯ̧�Ɏ�M�ް�������������ް��̓��e��������x��͂��ω�������Βʒm����		   |*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	LKcom_RcvDataGet( ushort *cnt )											/*								*/
{																				/*								*/
																				/*								*/
t_LKcomTest		*lk_tst;														/*								*/
t_LKcomMnt		*lk_mnt;														/*								*/
t_LKcomSubLock	*lk_lock2;														/*								*/
t_LKcomEcod		*lk_ecod;														/*								*/

																				/*								*/
																				/*								*/
ushort	d_siz;																	/* �ް�����						*/
uchar	d_id;																	/* �ް�ID						*/
uchar	tno;																	/* �[����						*/
uchar	ret;																	/*								*/
ushort	i, j;																	/*								*/
																				/* 0:�ω��Ȃ�					*/
																				/* 1:�ω�����(��������)			*/
																				/* 2:�ω�����(�����Ȃ�)			*/
																				/*								*/
union {																			/*								*/
	uchar	uc[2];																/*								*/
	ushort	us;																	/*								*/
}u;																				/*								*/

ushort	lkmax = FLAP_IFS_MAX;
																				/*								*/
	ret = 0;																	/*								*/
	if( LKcom_RcvData.dcnt == 0 ){												/* ��M�ް��Ȃ�					*/
		return( ret );															/* �������Ȃ��Ŕ�����			*/
	}																			/*								*/
																				/*								*/
	u.uc[0] = LKcom_RcvData.data[0];											/* �ް����ގ擾					*/
	u.uc[1] = LKcom_RcvData.data[1];											/*								*/
	d_siz = u.us;																/*								*/

// data[2]=ID,[3]=�f�[�^�ێ�,[4]�^�[�~�i��No,[5]�ȍ~�f�[�^
	tno = (uchar)(LKcom_RcvData.data[4]);										/* �[����						*/
	d_id = LKcom_RcvData.data[2];												/* �ް�ID4�擾					*/
	memcpy( &LKcom_RcvDtWork[0], &LKcom_RcvData.data[2],(size_t)((ushort)(d_siz-2)) );	/*						*/
																				/*								*/
	if( d_id <= LK_RCV_ECOD_ID ){												/* CRR/CRA����̎�M�d��		*/
		if(LOCK_REQ_ALL_TNO == tno){// ���b�N���u�S�Ăɑ΂���v���̌��ʂ̏ꍇ
		// �ʏ��ԃf�[�^�i�S�āj�A�����e�i���X���v�������A�o�[�W�����v�������̏ꍇ
			lkmax = LK_LOCK_MAX;												//�ڑ����u�ő吔(ۯ����u��90)
		}
		else if( FLAP_REQ_ALL_TNO == tno) {
			lkmax = INT_FLAP_MAX;											//1����ق̐ڑ����u�ő吔(����Flap���u��10)
		}else if( LKcom_Search_Ifno( (uchar)tno ) ){
			if(!( GetCarInfoParam() & 0x04 )){									/* CRR��ڑ���������Δ�����	*/
				LKcom_RcvDataDel();												/* ��M�ް��폜					*/
				return( 8 );
			}else{
				lkmax = INT_FLAP_MAX;											//1����ق̐ڑ����u�ő吔(����Flap���u��10)
			}
		}else{
			lkmax = LK_LOCK_MAX;												//�ڑ����u�ő吔(ۯ����u��90)
		}
	}
	switch( d_id )																/*								*/
	{																			/*								*/
																				/*								*/
	case LK_RCV_COND_ID:														/* ����ް�ID(��)				*/
		memset( &LockBuf, 0xff, sizeof( t_LKBUF )*lkmax );						// 0xffff�ŏ���������
		*cnt = 0;
		if( LKcom_RcvDtWork[3] != 0 ){											// ��ԃf�[�^�̃f�[�^��(�Ԏ���)�L
			for( i=0; i<LKcom_RcvDtWork[3]; i++ ){								// ���M����Ă�����񕪃��[�v
				// ���b�N���u
				//NOTE:LCM_NoticeStatus_toPAY�܂���LCM_NoticeStatusall_toPAY�Ńf�[�^�̓Z�b�g�����
				lk_lock2 = (t_LKcomSubLock *)&LKcom_RcvDtWork[4+(3*i)];
				if( (lk_lock2->lock_cnd == 7) || (lk_lock2->lock_cnd == 8) ){	/*								*/
					// ��M�ް����u�蓮Ӱ�ޒ��v�܂��́u�蓮Ӱ�މ����v
					LockBuf[*cnt].LockNoBuf = (ushort)tno;						/* ���uNo.�������No.���		*/
				}																/*								*/
				else{															/*								*/
					LockBuf[*cnt].LockNoBuf = (ushort)(lk_lock2->lock_no);		// LockInfo�̃C���f�b�N�X���Z�b�g
				}
				LockBuf[*cnt].car_cnd  = lk_lock2->car_cnd;
				LockBuf[*cnt].lock_cnd  = lk_lock2->lock_cnd;
				*cnt += 1;
			}
		}
		if( *cnt )	ret = 2;
		LKcom_RcvDataDel();														// ��M�ް��폜
		break;
																				/*								*/
	case LK_RCV_TEST_ID:														/* ý��ް�ID					*/
		lk_tst = (t_LKcomTest *)&LKcom_RcvDtWork;								/*								*/
		memcpy( &MntLockTest[0],&lk_tst->lock_tst[0], (size_t)lkmax );			/* ۯ����u�J��				*/
		queset( OPETCBNO, CTRL_PORT_CHK_RECV, 0, NULL );
		LKcom_RcvDataDel();														/* ��M�ް��폜					*/
		ret = 3;																/*								*/
		break;																	/*								*/
																				/*								*/
	case LK_RCV_MENT_ID:														/* ����ݽ�ް�ID					*/
		lk_mnt = (t_LKcomMnt *)&LKcom_RcvDtWork[3];	
		// �ް���M�p�ϊ�ð��ٍ쐬
		// ��MIF��No.��ۯ����u�A�Ԗ��ɐڑ��L���̐ݒ���������ڑ�����Ȃ瑕�uNo.��ϊ�ð��ق־�Ă���
		memset( MntLockWkTbl, 0, sizeof( MntLockWkTbl ) );	// �ް���M�p�ϊ�ð���0�ر

		if( lkmax == LK_LOCK_MAX ){// ����
			for( i=0 ; i<BIKE_LOCK_MAX ; i++ ){				// ۯ����u�A�ԍő�܂Ŏ�M�ް�������
				MntLockDoCount[i][0]  = *((ulong*)lk_mnt->lock_mnt[i].auto_cnt);		// ����񐔾���
				MntLockDoCount[i][1]  = *((ulong*)lk_mnt->lock_mnt[i].manu_cnt);		// �蓮�񐔾���
				MntLockDoCount[i][2]  = *((ulong*)lk_mnt->lock_mnt[i].trbl_cnt);		// �̏�񐔾���
			}
		}
		else if( lkmax == INT_FLAP_MAX ) {	// �t���b�v
			for( i=INT_CAR_START_INDEX, j=0 ; i< INT_CAR_START_INDEX + INT_FLAP_MAX ; i++, j++ ){	// �t���b�v�A�ԍő�܂Ŏ�M�ް�������
				MntFlapDoCount[i][0]  = *((ulong*)lk_mnt->lock_mnt[i].auto_cnt);		// ����񐔾���
				MntFlapDoCount[i][1]  = *((ulong*)lk_mnt->lock_mnt[i].manu_cnt);		// �蓮�񐔾���
				MntFlapDoCount[i][2]  = *((ulong*)lk_mnt->lock_mnt[i].trbl_cnt);		// �̏�񐔾���
			}
		}
		LKcom_RcvDataDel();														/* ��M�ް��폜					*/
		ret = 4;																/*								*/
		break;																	/*								*/
																				/*								*/
																				/*								*/
	case LK_RCV_ECOD_ID:														/*								*/
		lk_ecod = (t_LKcomEcod *)&LKcom_RcvDtWork;								/*								*/
		LKopeErrRegist( tno, lk_ecod );											/*								*/
		LKcom_RcvDataDel();														/* ��M�ް��폜					*/
		ret = 7;																/*								*/
		break;																	/*								*/
	case CRR_RCV_TEST_ID:
		queset( OPETCBNO, CTRL_PORT_CHK_RECV, 0, NULL );						/* ��M������`����				*/
		LKcom_RcvDataDel();														/* ��M�ް��폜					*/
		ret = 9;																/*								*/
		break;																	/*								*/
	default:																	/* ����ȊO��ID�̏ꍇ			*/
		/* ���肦�Ȃ������è�@�\������ */
		LKcom_RcvDataDel();														/* ��M�ް��폜					*/
		ret = 8;																/*								*/
		break;																	/*								*/
																				/*								*/
	}																			/*								*/
	return( ret );																/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	��M�ςݏ���ް���͏���																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKcom_RcvDataAnalys()																		   |*/
/*| PARAMETER	: par1	: ��M�����ް���																	   |*/
/*| RETURN VALUE: ret	: �ݒ�l																			   |*/
/*| 			: 		: -1 = �����װ																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: R.HARA(AMANO)																				   |*/
/*| Date		: 2005-09-12																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �E���ع���݂ֈ����n���p�ޯ̧�Ɏ�M�ް�������������ް��̓��e��������x��͂��ω�������Βʒm����		   |*/
/*|																											   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
uchar	LKcom_RcvDataAnalys( ushort cnt )										/*								*/
{																				/*								*/
	flp_com		*lk_tbl;														/*								*/
	ushort		nst_bak, ost_bak;												/* ���ð�����ޯ�����			*/
	uchar		wk_hit;															/* ���u���̏�Ԃ̕ω��׸�		*/
	uchar		ret;															/*								*/
	ulong		lkNo_w;															/* �ڋq�pۯ����u��				*/
	uchar		tno_wk;
	uchar		lk_kind;
	uchar		tno_max;
	uchar		set_mode;
	ushort		moni_no;
	uchar		InOutLog=0;
																				/*								*/
																				/*								*/
	ret = 0;																	/*								*/
	wk_hit = 0;																	/*								*/
																				/*								*/
	lk_tbl = &FLAPDT.flp_data[LockBuf[cnt].LockNoBuf];							/*								*/
	nst_bak = lk_tbl->nstat.word;												/* ���ð�����ޯ�����			*/
	ost_bak = lk_tbl->ostat.word;												/* ���ð�����ޯ�����			*/
	lk_tbl->ostat.word = lk_tbl->nstat.word;									/* ���ð�������ð���ɺ�߰		*/
	lkNo_w = (ulong)( LockInfo[LockBuf[cnt].LockNoBuf].area*10000L +			/*								*/
					  LockInfo[LockBuf[cnt].LockNoBuf].posi );					/* �ڋq�pۯ����u��				*/
																				/*								*/
	if( lk_tbl->nstat.bits.b09 && LockBuf[cnt].lock_cnd != 0 ){					/* NT-NET�׸�ON					*/
		if( lk_tbl->nstat.bits.b15 == 1 ){
			NTNET_Snd_Data05( lkNo_w, LockBuf[cnt].car_cnd, LockBuf[cnt].lock_cnd );/* NT-NETۯ����䌋���ް��쐬	*/
			lk_tbl->nstat.bits.b14 = 0;
		}
		lk_tbl->nstat.bits.b09 = 0;												/* NT-NET�׸�OFF				*/
	}																			/*								*/

	/*----------------*/
	/* �蓮Ӱ�ނ����� */
	/*----------------*/
	if( (LockBuf[cnt].lock_cnd == 7) || (LockBuf[cnt].lock_cnd == 8) ){			/* �u�蓮Ӱ�ޔ����vor�u�蓮Ӱ�މ����v�H	*/

		tno_wk = (uchar)LockBuf[cnt].LockNoBuf;

		if( (tno_wk & 0x80) == 0 ){												/* ���u��ʁH							*/

			// �ׯ�ߑ��u
			lk_kind = LK_KIND_FLAP;												/* ���u��ʁi�ׯ�߁j					*/
			tno_max = FLAP_IF_MAX;												/* �ׯ�ߑ��u�����No.�ő�l				*/

			if( LockBuf[cnt].lock_cnd == 7 ){
				// �蓮Ӱ�ޔ���
				set_mode = ON;													/* �蓮Ӱ���׸ލX�V�ް��iON�j���		*/
				moni_no  = OPMON_FLAP_MMODE_ON;									/* �������No.�i�ׯ�ߑ��u�蓮Ӱ�ޔ����j	*/
			}
			else{
				// �蓮Ӱ�މ���
				set_mode = OFF;													/* �蓮Ӱ���׸ލX�V�ް��iOFF�j���		*/
				moni_no  = OPMON_FLAP_MMODE_OFF;								/* �������No.�i�ׯ�ߑ��u�蓮Ӱ�މ����j	*/
			}
		}
		else{
			// ۯ����u
			lk_kind = LK_KIND_LOCK;												/* ���u��ʁiۯ����u�j					*/
			tno_max = LOCK_IF_MAX;												/* ۯ����u�����No.�ő�l				*/
			tno_wk = (uchar)(tno_wk & 0x7f);									/* �����No.�ϊ�							*/
			if( LKcom_Search_Ifno( tno_wk )){
				lk_kind = LK_KIND_INT_FLAP;										/* ���u��ʁiۯ����u�j					*/
			}
			if( LockBuf[cnt].lock_cnd == 7 ){
				// �蓮Ӱ�ޔ���
				set_mode = ON;													/* �蓮Ӱ���׸ލX�V�ް��iON�j���		*/
				moni_no  = OPMON_LOCK_MMODE_ON;									/* �������No.�iۯ����u�蓮Ӱ�ޔ����j	*/
				moni_no  = ( lk_kind == LK_KIND_INT_FLAP ? 
							OPMON_FLAP_MMODE_ON:OPMON_LOCK_MMODE_ON);			/* �������No.�iۯ����u�蓮Ӱ�ޔ����j	*/
			}
			else{
				// �蓮Ӱ�މ���
				set_mode = OFF;													/* �蓮Ӱ���׸ލX�V�ް��iOFF�j���		*/
				moni_no  = OPMON_LOCK_MMODE_OFF;								/* �������No.�iۯ����u�蓮Ӱ�މ����j	*/
				moni_no  = ( lk_kind == LK_KIND_INT_FLAP ? 
							OPMON_FLAP_MMODE_OFF:OPMON_LOCK_MMODE_OFF);			/* �������No.�iۯ����u�蓮Ӱ�ޔ����j	*/
			}
		}
		if( (tno_wk >= 1) && (tno_wk <= tno_max) ){								/* �����No.����							*/

			if( lk_kind == LK_KIND_FLAP ){										/* ���u��ʁH							*/
				// �ׯ�ߑ��u
				flp_m_mode[tno_wk-1] = set_mode;								/* �ׯ�ߑ��u�蓮Ӱ���׸ލX�V			*/
			}
			else{
				// ۯ����u
				lok_m_mode[tno_wk-1] = set_mode;								/* ۯ����u�蓮Ӱ���׸ލX�V				*/
			}
			wmonlg( moni_no, NULL, (ulong)tno_wk );								/* ����o�^�F�蓮Ӱ�ޔ����^����			*/
			queset( OPETCBNO, OPE_OPNCLS_EVT, 0, NULL );						/* OpeMain�֒ʒm�i�c�Ɓ^�x�Ɛؑ������j	*/
		}
		return( ret );
	}

/*----------------------------------------------------------------------*/
/* �ڑ��Ȃ��́u�ԗ����m��ԁv�Ɓuۯ����u��ԁv�̗����ɑ��݂���			*/
/* �ǂ��炩������ڑ��Ȃ��̏ꍇ�ɂ́u�ڑ��Ȃ��v�Ɣ��f����B				*/
/* �ڑ��Ȃ��̏ꍇ�ɂ͐ڑ��Ȃ�bit��ON�ɂ��A���̑��̏��͑S�ĸر����		*/
/* ���̎����ۯ��A�Jۯ��ł���Ε�ۯ��A�Jۯ�����������B					*/
/* ���ɏ�Ԃ̏ꍇ�ɂ͏o�Ɉ����i�����o�Ɂj�Ƃ���B						*/
/* ۯ����u��Ԃł���ΊJ��ԁi��̫�āj�Ƃ���B						*/
/*----------------------------------------------------------------------*/
																				/*								*/
	/*----------------*/
	/* �ڑ��L�������� */
	/*----------------*/
	if( (LockBuf[cnt].car_cnd == 0 )||											/* �ԗ��ڑ���ԁ�[�ڑ��Ȃ�]		*/
		(LockBuf[cnt].lock_cnd == 6 )) 											/* ۯ����u��� ��[�ڑ��Ȃ�]		*/
	{																			/*								*/
		if( lk_tbl->ostat.bits.b08 == 0 )										/* ���F�ڑ�����(���聨�Ȃ�)		*/
		{																		/*								*/
			lk_tbl->nstat.word |= 0x0100;										/* ���F�ڑ��Ȃ�					*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
			/*--------------------------------------*/
			/* �װ�󎚁F�ڑ��Ȃ�����(�e�@������u��) */
			/*--------------------------------------*/
		}																		/*								*/
		else																	/* ���F�ڑ��Ȃ�(�Ȃ����Ȃ�)		*/
		{																		/*								*/
			lk_tbl->nstat.word = nst_bak;										/* ��Ԃ̕ύX���Ȃ�				*/
			lk_tbl->ostat.word = ost_bak;										/* ��Ԃ̕ύX���Ȃ�				*/
		}																		/*								*/
		return( ret );															/* �ω�����̎��͏���������Ȃ�	*/
	}																			/* �ڑ��Ȃ��Ȃ̂ňȌ�������Ȃ�	*/
	else																		/* [�ڑ�����]					*/
	{																			/*								*/
		if( lk_tbl->ostat.bits.b08 == 1 )										/* ���F�ڑ��Ȃ�(�Ȃ�������)		*/
		{																		/*								*/
			lk_tbl->nstat.bits.b08 = 0;											/* ���F�ڑ�����					*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
			/*--------------------------------------*/
			/* �װ�󎚁F�ڑ��Ȃ�����(�e�@������u��) */
			/*--------------------------------------*/
		}																		/*								*/
	}																			/*								*/
																				/*								*/
																				/*								*/
	/*--------------*/
	/* �ԗ����m��� */
	/*--------------*/
	switch( LockBuf[cnt].car_cnd )												/* �ԗ����m��Ԃ�����			*/
	{																			/*								*/
	case 1:																		/* [�ԗ�����]					*/
		if( lk_tbl->ostat.bits.b00 == 0 ){										/* ���F�ԗ��Ȃ�					*/
			/* �ԗ��Ȃ����ԗ����� */											/*								*/
			lk_tbl->nstat.bits.b00 = 1;											/* ���F�ԗ�����					*/
			if( lk_tbl->nstat.bits.b03 == 1 ){									/*								*/
				lk_tbl->nstat.bits.b03 = 0;										/* ���F�����o�ɉ���				*/
			}																	/*								*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
		}else{																	/* ���F�ԗ�����					*/
			/* �ԗ����聨�ԗ����� */											/*								*/
			if( lk_tbl->ostat.bits.b03 == 1 ){									/* ���F�����o�ɒ�				*/
				lk_tbl->nstat.bits.b03 = 0;										/* ���F�����o�ɉ���				*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	case 2:																		/* [�ԗ��Ȃ�]					*/
		if( lk_tbl->ostat.bits.b00 == 1 ){										/* ���F�ԗ�����					*/
			/* �ԗ����聨�ԗ��Ȃ� */											/*								*/
			lk_tbl->nstat.bits.b00 = 0;											/* ��:�ԗ��Ȃ�					*/
			if( lk_tbl->nstat.bits.b03 == 1 ){									/*								*/
				lk_tbl->nstat.bits.b03 = 0;										/* ���F�����o�ɉ���				*/
			}																	/*								*/

			// �ԗ����� �� �Ȃ��̂Ƃ��A��Ԉُ��׸޸ر
			lk_tbl->nstat.bits.b12 = 0;		// �㏸�ُ��
			lk_tbl->nstat.bits.b13 = 0;		// ���~�ُ��

			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
		}else{																	/* ���F�ԗ��Ȃ�					*/
			/* �ԗ��Ȃ����ԗ��Ȃ� */											/*								*/
			if( lk_tbl->ostat.bits.b03 == 1 ){									/* ���F�����o�ɒ�				*/
				lk_tbl->nstat.bits.b03 = 0;										/* ���F�����o�ɉ���				*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
		}																		/*								*/
		break;																	/*								*/
	case 3:																		/* �蓮Ӱ�ނŋ����o��			*/
		if( lk_tbl->ostat.bits.b03 == 0 ){										/* ��:�����o�ɒ��łȂ�			*/
			lk_tbl->nstat.bits.b03 = 1;											/* ��:�����o��					*/
			if( lk_tbl->ostat.bits.b00 == 1 ){									/*								*/
				lk_tbl->nstat.bits.b00 = 0;										/* ��:�ԗ��Ȃ�					*/

				// �ԗ����� �� �Ȃ��̂Ƃ��A��Ԉُ��׸޸ر
				lk_tbl->nstat.bits.b12 = 0;		// �㏸�ُ��
				lk_tbl->nstat.bits.b13 = 0;		// ���~�ُ��
			}																	/*								*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
		}																		/*								*/
		break;																	/*								*/
	}																			/*								*/
	/*--------------*/
	/* ۯ����u���	*/
	/*--------------*/
	switch( LockBuf[cnt].lock_cnd )												/*								*/
	{																			/*								*/
	case 1:																		/* �㏸�ς�(ۯ����u�ς�)		*/
		if( lk_tbl->ostat.bits.b01 == 0 )										/* ���~�ς�(ۯ����u�J�ς�)		*/
		{																		/*								*/
			/* ���~�ς݁��㏸�ς� */											/*								*/
			lk_tbl->nstat.bits.b01 = 1;											/* �㏸�ς�(ۯ����u�ς�)		*/
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPUP, NULL, lkNo_w );							/* ����o�^(�ׯ�ߕ�)				*/
			} else {
				wmonlg( OPMON_LOCKUP, NULL, lkNo_w );							/* ����o�^(ۯ���)				*/
			}
			if( lk_tbl->ostat.bits.b04 == 1 )									/* �㏸ۯ���(��ۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* �㏸ۯ�����(��ۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŕ�ۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* ���~ۯ���(�Jۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* ���~ۯ�����(�Jۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŊJۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
			InOutLog = 1;														/* ���o�Ƀ��O�o�^����(�㏸/��)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			/* �㏸�ς݁��㏸�ς� */											/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* �㏸ۯ���(��ۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* �㏸ۯ�����(��ۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŕ�ۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
				InOutLog = 1;													/* ���o�Ƀ��O�o�^����(�㏸/��)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* ���~ۯ���(�Jۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* ���~ۯ�����(�Jۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŊJۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
				InOutLog = 1;													/* ���o�Ƀ��O�o�^����(�㏸/��)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b07 == 1 ){
				FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* �ׯ�ߓ���Ď���ϰ��~		*/
			}
			if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){						/* ���񏈗�						*/
				// �ԗ��Ȃ��Ȃ�A��ԕω�����Ƃ���
				if (lk_tbl->nstat.bits.b00 == 0) {								/* �ԗ��Ȃ�						*/
					ret = 2;													/* ��ԕω�����					*/
					wk_hit = 1;													/*								*/
					break;														/*								*/
				}																/*								*/
			}																	/*								*/
			else if( DownLockFlag[LockBuf[cnt].LockNoBuf] == 1 ){				/* ���񏈗��ς�					*/
				DownLockFlag[LockBuf[cnt].LockNoBuf]++;							/* 2��ڈȍ~�̏�ԕω�			*/
			}																	/*								*/
		}																		/*								*/
		lk_tbl->nstat.bits.b12 = 0;		// �㏸�ُ��
		break;																	/*								*/
	case 2:																		/* ���~�ς�(ۯ����u�J�ς�)		*/
		if( lk_tbl->ostat.bits.b01 == 1 )										/* �㏸�ς�(ۯ����u�ς�)		*/
		{																		/*								*/
			/* �㏸�ς݁����~�ς� */											/*								*/
			lk_tbl->nstat.bits.b01 = 0;											/* ���~�ς�(ۯ����u�J�ς�)		*/
// �s��C��(S) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
			FurikaeDestFlapNo = 0;
// �s��C��(E) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPDOWN, NULL, lkNo_w );							/* ����o�^(�ׯ�ߕ�)				*/
			} else {
				wmonlg( OPMON_LOCKDOWN, NULL, lkNo_w );							/* ����o�^(ۯ���)				*/
			}
			if( lk_tbl->ostat.bits.b04 == 1 )									/* �㏸ۯ���(��ۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* �㏸ۯ�����(��ۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŕ�ۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* ���~ۯ���(�Jۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* ���~ۯ�����(�Jۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŊJۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
			InOutLog = 2;														/* ���o�Ƀ��O�o�^����(���~/�J)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			/* ���~�ς݁����~�ς� */											/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* �㏸ۯ���(��ۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* �㏸ۯ�����(��ۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŕ�ۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
				InOutLog = 2;													/* ���o�Ƀ��O�o�^����(���~/�J)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* ���~ۯ���(�Jۯ���)			*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* ���~ۯ�����(�Jۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŊJۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
				InOutLog = 2;													/* ���o�Ƀ��O�o�^����(���~/�J)	*/
			}																	/*								*/
			if( lk_tbl->ostat.bits.b07 == 1 ){
				FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* �ׯ�ߓ���Ď���ϰ��~		*/
			}
		}																		/*								*/
		lk_tbl->nstat.bits.b13 = 0;		// ���~�ُ��
		break;																	/*								*/
																				/*								*/
	case 3:																		/* �㏸ۯ�(����ُ�)			*/
		if( lk_tbl->ostat.bits.b04 == 0 )										/* �㏸ۯ��Ȃ�(��ۯ��Ȃ�)		*/
		{																		/*								*/
			lk_tbl->nstat.bits.b01 = 1;											/* �㏸�ς�(ۯ����u�ς�)		*/
			lk_tbl->nstat.bits.b04 = 1;											/* �㏸ۯ�(��ۯ�)				*/
			// �㏸ۯ������A�����ݎԗ������݂���Ȃ�A�㏸�ُ��׸�ON
			if (lk_tbl->nstat.bits.b00 == 1) {
				lk_tbl->nstat.bits.b12 = 1;		// �㏸�ُ픭��
			}
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPUP, NULL, lkNo_w );							/* ����o�^(�ׯ�ߕ�)				*/
			} else {
				wmonlg( OPMON_LOCKUP, NULL, lkNo_w );							/* ����o�^(ۯ���)				*/
			}
			/*--------------------*/
			/* �װ�����F��ۯ����� */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 1 );			/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* ���~ۯ�����(�Jۯ�����)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* ���~ۯ�����(�Jۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŊJۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
			InOutLog = 3;														/* ���o�Ƀ��O�o�^����(�㏸/���b�N)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			if( lk_tbl->ostat.bits.b05 == 1 )									/* ���~ۯ�����(�Jۯ�����)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b05 = 0;										/* ���~ۯ�����(�Jۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŊJۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );		/*								*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
			FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* �ׯ�ߓ���Ď���ϰ��~		*/
		}																		/*								*/
		break;																	/*								*/
																				/*								*/
	case 4:																		/* ���~ۯ�(�J����ُ�)			*/
		if( lk_tbl->ostat.bits.b05 == 0 )										/* ���~ۯ��Ȃ�(�Jۯ��Ȃ�)		*/
		{																		/*								*/
			if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){						/* ���񏈗�						*/
				// �ԗ��Ȃ��ŉ��~ۯ��̏ꍇ�͏㏸�ς݂Ƃ��Ĉ���
				if (lk_tbl->nstat.bits.b00 == 0) {								/* �ԗ��Ȃ�						*/
					lk_tbl->nstat.bits.b01 = 1;									/* �㏸�ς�(ۯ����u�ς�)		*/
				}																/*								*/
			}																	/*								*/
			else if( DownLockFlag[LockBuf[cnt].LockNoBuf] == 1 ){				/* ���񏈗��ς�					*/
				lk_tbl->nstat.bits.b01 = 0;										/* ���~�ς�(ۯ����u�J�ς�)		*/
// �s��C��(S) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
				FurikaeDestFlapNo = 0;
// �s��C��(E) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
				DownLockFlag[LockBuf[cnt].LockNoBuf]++;							/* 2��ڈȍ~�̏�ԕω�			*/
			}																	/*								*/
			lk_tbl->nstat.bits.b05 = 1;											/* ���~ۯ�(�Jۯ�)				*/
			// ���~ۯ������A�����ݎԗ������݂���Ȃ�A���~�ُ��׸�ON
			if (lk_tbl->nstat.bits.b00 == 1) {
				lk_tbl->nstat.bits.b13 = 1;		// ���~�ُ픭��
			}
			if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
				wmonlg( OPMON_FLAPDOWN, NULL, lkNo_w );							/* ����o�^(�ׯ�ߕ�)				*/
			} else {
				wmonlg( OPMON_LOCKDOWN, NULL, lkNo_w );							/* ����o�^(ۯ���)				*/
			}
			/*--------------------*/
			/* �װ�����F�Jۯ����� */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 1 );			/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* �㏸ۯ�����(��ۯ�����)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* �㏸ۯ�����(��ۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŕ�ۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
			}																	/*								*/
			ret = 2;															/* ��ԕω�����					*/
			wk_hit = 1;															/*								*/
			InOutLog = 4;														/* ���o�Ƀ��O�o�^����(���~/�J���b�N)	*/
		}																		/*								*/
		else																	/*								*/
		{																		/*								*/
			if( lk_tbl->ostat.bits.b04 == 1 )									/* �㏸ۯ�����(��ۯ�����)		*/
			{																	/*								*/
				lk_tbl->nstat.bits.b04 = 0;										/* �㏸ۯ�����(��ۯ�����)		*/
				/*--------------------------------------*/
				/* �װ�����F�̏������M�Ȃ��ŕ�ۯ����� */
				/*--------------------------------------*/
				lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );		/*								*/
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
			}																	/*								*/
			FmvTimer( (ushort)(LockBuf[cnt].LockNoBuf + 1), -1 );				/* �ׯ�ߓ���Ď���ϰ��~		*/
			if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){
				// ���~���b�N�������́A��ԕω����������悤�ɐU����
				ret = 2;														/* ��ԕω�����					*/
				wk_hit = 1;														/*								*/
				DownLockFlag[LockBuf[cnt].LockNoBuf] = 1;						/* ���񏈗��ς�					*/
			}
		}																		/*								*/
		break;																	/*								*/
	case 5:																		/* �̏����						*/
		if( lk_tbl->ostat.bits.b04 == 1 ){										/* �㏸ۯ�(��ۯ�)				*/
			lk_tbl->nstat.bits.b04 = 0;											/* �㏸ۯ�(��ۯ�)����			*/
			/*--------------------*/
			/* �װ�����F��ۯ����� */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKCLOSEFAIL, 0 );			/*								*/
		}																		/*								*/
		if( lk_tbl->ostat.bits.b05 == 1 ){										/* ���~ۯ�(�Jۯ�)				*/
			lk_tbl->nstat.bits.b05 = 0;											/* ���~ۯ�(�Jۯ�)����			*/
			/*--------------------*/
			/* �װ�����F�Jۯ����� */
			/*--------------------*/
			lk_err_chk( LockBuf[cnt].LockNoBuf, ERR_LOCKOPENFAIL, 0 );			/*								*/
		}																		/*								*/
		if( lk_tbl->ostat.bits.b02 == 1 )										/* �㏸����(����)				*/
		{																		/*								*/
			lk_tbl->nstat.bits.b01 = 1;											/* �㏸�ς�(ۯ����u�ς�)		*/
			if( lk_tbl->ostat.bits.b05 == 1 ){									/* ���~ۯ�(�Jۯ�)				*/
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					wmonlg( OPMON_FLAPUP, NULL, lkNo_w );						/* ����o�^(�ׯ�ߕ�)				*/
				} else {
					wmonlg( OPMON_LOCKUP, NULL, lkNo_w );						/* ����o�^(ۯ���)				*/
				}
			}
			InOutLog = 1;														/* ���o�Ƀ��O�o�^����(�㏸/��)	*/
		}																		/*								*/
		else																	/* ���~����(�J����)				*/
		{																		/*								*/
			lk_tbl->nstat.bits.b01 = 0;											/* ���~�ς�(ۯ����u�J�ς�)		*/
// �s��C��(S) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
			FurikaeDestFlapNo = 0;
// �s��C��(E) K.Onodera 2016/12/09 #1582 �U�֌��̃t���b�v���㏸���ɓd����؂�ƁA�U�֐�̃t���b�v�����~���Ȃ��Ȃ�
			if( lk_tbl->ostat.bits.b04 == 1 ){									/* �㏸ۯ�(��ۯ�)				*/
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					wmonlg( OPMON_FLAPDOWN, NULL, lkNo_w );						/* ����o�^(�ׯ�ߕ�)				*/
				} else {
					wmonlg( OPMON_LOCKDOWN, NULL, lkNo_w );						/* ����o�^(ۯ���)				*/
				}
			}
			InOutLog = 2;														/* ���o�Ƀ��O�o�^����(�㏸/��)	*/
		}																		/*								*/
		ret = 2;																/* ��ԕω�����					*/
		wk_hit = 1;																/*								*/
		break;																	/*								*/
	}																			/*								*/
	if( !DownLockFlag[LockBuf[cnt].LockNoBuf] ){								/* �N�����̏����ԃf�[�^����M */
		// ���~���b�N��ԏ����͋N�����ɉ��~���b�N�łȂ���΁A�ȍ~���~���b�N�ƂȂ��Ă��������Ȃ��i���܂Œʂ�Ƃ���j
		DownLockFlag[LockBuf[cnt].LockNoBuf] = 1;								/* ���񏈗��ς�					*/
	}
	if( wk_hit != 1 ){															/* ��Ԃ̕ω��Ȃ�				*/
		LockBuf[cnt].LockNoBuf = 0xfffe;										/* ��ԕω��Ȃ����				*/
		if( wk_hit != 2 ){														/*								*/
			lk_tbl->nstat.word = nst_bak;										/* ���ð�������ɖ߂�			*/
			lk_tbl->ostat.word = ost_bak;										/* ���ð�������ɖ߂�			*/
		}																		/*								*/
	}																			/*								*/

	// (InOutLog != 0) �̏ꍇ�� (wk_hit == 1) �ƂȂ�B
	// ���̂��� (InOutLog != 0) �̏ꍇ�� (LockBuf[cnt].LockNoBuf = 0xfffe) �Ƃ͂Ȃ�Ȃ����߁A
	// �ȉ������͖��Ȃ��B
	if( InOutLog ){																/* ���o�Ƀ��O�o�^����			*/
		lkNo_w = LockInfo[LockBuf[cnt].LockNoBuf].posi;
		switch( InOutLog ){
			case	1:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* �t���b�v�㏸�������o�^ */
					IoLog_write(IOLOG_EVNT_FLAP_UP_FIN, (ushort)lkNo_w, 0, 0);
				} else {
					/* ���փ��b�N�������o�^ */
					IoLog_write(IOLOG_EVNT_LOCK_CL_FIN, (ushort)lkNo_w, 0, 0);
				}
				break;
			case	2:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* �t���b�v���~�������o�^ */
					IoLog_write(IOLOG_EVNT_FLAP_DW_FIN, (ushort)lkNo_w, 0, 0);
				} else {
					/* ���փ��b�N�J�������o�^ */
					IoLog_write(IOLOG_EVNT_LOCK_OP_FIN, (ushort)lkNo_w, 0, 0);
				}
				break;
			case	3:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* �t���b�v�㏸�������o�^ */
					IoLog_write(IOLOG_EVNT_FLAP_UP_UFN, (ushort)lkNo_w, 0, 0);
				} else {
					/* ���b�N���얢�����o�^ */
					IoLog_write(IOLOG_EVNT_LOCK_CL_UFN, (ushort)lkNo_w, 0, 0);
				}
				break;
			case	4:
				if (LockBuf[cnt].LockNoBuf < FLAP_NUM_MAX) {
					/* �t���b�v���~�������o�^ */
					IoLog_write(IOLOG_EVNT_FLAP_DW_UFN, (ushort)lkNo_w, 0, 0);
				} else {
					/* ���b�N�J���얢�����o�^ */
					IoLog_write(IOLOG_EVNT_LOCK_OP_UFN, (ushort)lkNo_w, 0, 0);
				}
				break;
		}
	}
	return( ret );																/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF�Րݒ���ð��ٍ쐬(API)																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKInti_SetLockMaker()																		   |*/
/*| PARAMETER	: NON	:																					   |*/
/*| RETURN VALUE: ret	: 																					   |*/
/*| 			: 		: 																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �ELockInfoð��ق���e�@�ʂɎq�@����ۯ����u��ʂ��擾��������ޯ̧�֊i�[����B							   |*/
/*|	�E�d��ON���A�ݒ�ύX���ɕK����ق��邱��																	   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	LKcom_SetLockMaker( void )
{
ushort	i;
	memset( &child_mk, 0x00, sizeof( child_mk ) );
	for( i=CRR_CTRL_START; i<LOCK_MAX; i++ )
	{
		WACDOG;																	// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( LockInfo[i].if_oya != 0 )
		{
			if( (LockInfo[i].lok_syu != 0)&&									// ���b�N���u���
				(LockInfo[i].lok_no  != 0) ){									// �ڑ��^�[�~�i��No�ɑ΂��郍�b�N���u�A��
				child_mk[LockInfo[i].if_oya-1] = LockInfo[i].lok_syu;			// ���b�N���u��ʂ��Z�b�g
			}
		}
	}
}
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF�Ր���(API)																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeApiLKCtrl()																		   |*/
/*| PARAMETER	: NON	:																					   |*/
/*| RETURN VALUE: ret	: 																					   |*/
/*| 			: 		: 																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-02-23																				   |*/
/*| Update		: A.iiizumi 2011/11/10																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �E																										   |*/
/*|	�E																										   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	LKopeApiLKCtrl( ushort tb_no, uchar kind )								/* ۯ����u�w��					*/
{																				/*								*/
	ushort	lk_no	= 0;														/* ۯ����u�ԍ�					*/
	uchar	wk_tno	= 0;														/*								*/
#if (1 == AUTO_PAYMENT_PROGRAM)
	// ��а�֑��M(�̑���ɒʒm)����
	if (OPECTL.Seisan_Chk_mod == ON) {
		if (tb_no == 0) return;
		// �ׯ�ߑ��M
		if (tb_no <= BIKE_START_INDEX) {
			lk_no  = (ushort)(tb_no-1);
			wk_tno = LockInfo[lk_no].if_oya;
			LK_AutoPayment_Send(MAX_AUTO_PAYMENT_CAR, wk_tno, (uchar)1, kind);	// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
		}

		// ۯ����M
		if ((tb_no > INT_CAR_START_INDEX) && (tb_no <= LOCK_MAX)) {
			lk_no  = (ushort)(tb_no-1);
			wk_tno = LockInfo[lk_no].if_oya;
			// �ׯ�ߗp�d���쐬�֐��𗬗p���Ă��邽�߁A���uNo�͎Ԏ����Ұ�����擾����(lok_no)
			LK_AutoPayment_Send(MAX_AUTO_PAYMENT_CAR+1, wk_tno, (uchar)LockInfo[lk_no].lok_no, kind);
		}
		return;
	}
#endif

	if( kind == (uchar)(LK_SND_P_CHK&0x000f) ){
		// �|�[�g�����̏ꍇ�́A�Ԏ��ԍ��ł͂Ȃ��A����ٔԍ����n����Ă���B
		LKcom_SetDtPkt( LK_SND_CTRL_ID, (uchar)tb_no + IFS_CRR_OFFSET -1, 0, kind );	// CRR��փA�N�Z�X����ꍇ�A�^�[�~�i��No.�{IFS_CRR_OFFSET���邱�ƂŁA�^�[�~�i��No.�ƍ��ʉ�����
		return;
	}
	if (tb_no == 0) return;

	// ۯ����M
	if ((tb_no > CRR_CTRL_START) && (tb_no <= LOCK_MAX)) {
		lk_no  = (ushort)(tb_no-1);
		wk_tno = LockInfo[lk_no].if_oya;
		LKcom_SetDtPkt( LK_SND_CTRL_ID, wk_tno, tb_no, kind );
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: LKopeApiLKCtrl_All
 *[]----------------------------------------------------------------------[]
 *| summary	: �S�ׯ��/�Sۯ����u�ɓ����ɐ��亰�ނ𑗐M����
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
void	LKopeApiLKCtrl_All(uchar mtype, uchar kind)
{
	uchar	t_no_max, i;
	t_no_max = (uchar)LKcom_GetAccessTarminalCount();				// �t���b�v����ՁA���b�N���u�pIF�Վq�@�ڑ��䐔

	if ((mtype == _MTYPE_LOCK) ) {
		for (i = 1; i <= t_no_max; i++) {
			if(0 == LKcom_Search_Ifno( i )){
			// NOTE:�����ł�CRA��I/F�`���ł���A�^�[�~�i��No��CRB�P�ʂƂȂ�̂ň�ł�
			// CRB�ڑ��̒[��������������d�����Z�b�g���Ĕ�����
				LKcom_SetDtPkt(LK_SND_CTRL_ID, i, 0, kind);
				break;
			}
		}
	}
	if ((mtype == _MTYPE_INT_FLAP) ) {
		if( kind != 12 ){	// �o�[�W�������ȊO�̏ꍇ
			for (i = 1; i <= t_no_max; i++) {
				if(1 == LKcom_Search_Ifno( i )){
					LKcom_SetDtPkt(LK_SND_CTRL_ID, i, 0, kind);
					break;
				}
			}
		} else {			// �t���b�v�̃o�[�W��������CRR��ւ̃A�N�Z�X�ɂ���
			for (i=0; i < IFS_CRR_MAX; i++) {
				if ( 1 == IFM_LockTable.sSlave_CRR[i].bComm ){
					LKcom_SetDtPkt(LK_SND_CTRL_ID, i+IFS_CRR_OFFSET, 0, kind);	// CRR��փA�N�Z�X����ꍇ�A�^�[�~�i��No.�{IFS_CRR_OFFSET���邱�ƂŁA�^�[�~�i��No.�ƍ��ʉ�����
					break;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	IF�Տ��e�[�u���iLockInfo�j����(API)																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeGetLockNum()																			   |*/
/*| PARAMETER	: par1	: ���(1�`26)																		   |*/
/*| 			: par2	: ���Ԉʒu�ԍ�(1�`9999)																   |*/
/*| 			: par3	: LockInfo�z��ԍ�+1																   |*/
/*| RETURN VALUE: ret	: 1=�Y���Ԏ����� 0=�Y���Ԏ��Ȃ�														   |*/
/*| 			: 		: 																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Akiba(AMANO)																			   |*/
/*| Date		: 2005-05-18																				   |*/
/*| Update		:																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �E																										   |*/
/*|	�E																										   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
short	LKopeGetLockNum( uchar snum, ushort pnum, ushort *posi )				/*								*/
{																				/*								*/
	ushort	i;																	/*								*/
	uchar	work;																/*								*/
																				/*								*/
	work = GetCarInfoParam();													/*								*/
	for( i = 0; i < LOCK_MAX; i++ ){											/* 324�Ԏ�����					*/
		WACDOG;																// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
		if( i < INT_CAR_START_INDEX ) {											/*								*/
			if( !( work & 0x02 ) ) {											/*								*/
				continue;														/*								*/
			}																	/*								*/
		} else if( i < BIKE_START_INDEX ) {										/*								*/
			if( !( work & 0x04 ) ) {											/*								*/
				continue;														/*								*/
			}																	/*								*/
		} else {																/*								*/
			if( !( work & 0x01 ) ) {											/*								*/
				continue;														/*								*/
			}																	/*								*/
		}																		/*								*/
		if(( LockInfo[i].area == snum )&&										/* ���ƒ��Ԉʒu����v			*/
		   ( LockInfo[i].posi == pnum )){										/*								*/
			*posi = i+1;														/* LockInfo�z��ԍ�+1(1�`324)	*/
			return( 1 );														/* �Y���Ԏ�����					*/
		}																		/*								*/
	}																			/*								*/
	return( 0 );																/* �Y���Ԏ��Ȃ�					*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�G���[�f�[�^��M���̃G���[�o�^																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeErrRegist()																			   |*/
/*| PARAMETER	: tno	: �e�@�[����																		   |*/
/*| 			: lk_ecod: t_LKcomEcod�̃|�C���^															   |*/
/*| RETURN VALUE: ret	�Ȃ�																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																					   |*/
/*| Date		: 2005-08-08																				   |*/
/*| Update		:																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	LKopeErrRegist( uchar tno, t_LKcomEcod *lk_ecod )
{
	ushort	i;
	char occ;
	i = 0;
	if( IFS_CRR_OFFSET > lk_ecod->tno ){	// CRR��ւ̃A�N�Z�X�ł͂Ȃ�
		if( lk_ecod->tno > LOCK_IF_MAX ){ // �[���̍ő吔�ŃK�[�h
			 return;
		}
	} else {								// CRR��ւ̃A�N�Z�X�̏ꍇ�A100�`102�������L��
		if( lk_ecod->tno < IFS_CRR_OFFSET || lk_ecod->tno > IFS_CRR_OFFSET + 2 )
			 return;
	}
		
	switch( lk_ecod->err ){
		case	1:// �p���e�B�G���[
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_PARITYERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	2:// �t���[�~���O�G���[
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_FLAMERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	3:// �I�[�o�[�����G���[
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_OVERRUNERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	4:// �d���ُ�i�w�b�_�����j
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_HEADERERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	5:// �d���ُ�i�f�[�^��NG�j
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_LENGTHERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	6:// �d���ُ�i�f�[�^IDNG�j
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_IDERROR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	7:// NAK���g���C�I�[�o�[
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_NAKRYOVER, (char)lk_ecod->occr, 0, 0 );
			break;
		case	8:// ���M���g���C�I�[�o�[�i�ʐM�ُ�j
			if( lk_ecod->tno > 0 && lk_ecod->tno < IFS_CRR_OFFSET){	// CRR��̃��g���C�I�[�o�[�͕s�v�Ȃ̂ňꉞ�������Ă���
				// �q�@�̴װ
				if(IFM_LockTable.sSlave[tno - 1].bSomeone != 1) {	// �[���ڑ��Ȃ���
					break;
				}
				occ = (char)lk_ecod->occr;
				if( occ > 2 ){
					 occ = 0;
				}
				ErrBinDatSet( (ulong)lk_ecod->tno - 1, 0 );
				err_chk( ERRMDL_FLAP_CRB, (char)(ERR_FLAPLOCK_COMFAIL+tno-1), (char)occ, 2, 0 );
			}
			break;
		case	9:// CRC�ُ�
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_CRC_ERR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	10:// �d���ُ�i�w�b�_������M�j
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_NOHEADER, (char)lk_ecod->occr, 0, 0 );
			break;
		case	11:// �v���g�R���ُ�
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_PLOTOCOLERR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	14:// ����Ⴂ
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_RESERR, (char)lk_ecod->occr, 0, 0 );
			break;
		case	21:// �L���[�t���i�q�@��ԃ��X�g�j
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_QUEFULL1, (char)lk_ecod->occr, 0, 0 );
			break;
		case	22:// �L���[�t���i���b�N����҂����X�g�j
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_QUEFULL2, (char)lk_ecod->occr, 0, 0 );
			break;
		case	23:// �L���[�t���i�q�@�Z���N�e�B���O�҂����X�g�j
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_QUEFULL3, (char)lk_ecod->occr, 0, 0 );
			break;
		case	31:// ����f�[�^�ُ�
			ErrBinDatSet( (ulong)i, 1 );
			err_chk( ERRMDL_FLAP_CRB, ERR_FLAPLOCK_DATAERR, (char)lk_ecod->occr, 0, 0 );
			break;
	}
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���b�N���u�ɊJ�^���b�N���������Ă��邩�`�F�b�N														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: LKopeLockErrCheck()																		   |*/
/*| PARAMETER	: OpenCloose	: 0=���b�N�A1=�J���b�N													   |*/
/*| RETURN VALUE: ret	=0:�װ�Ȃ��A<>0:�������Ă��鐔														   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																					   |*/
/*| Date		: 2005-08-08																				   |*/
/*| Update		:																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
short	LKopeLockErrCheck( short OpenClose, uchar mtype )
{
	int	i, errnum;

	int	start, end;

	if (mtype == _MTYPE_LOCK) {
		start = FLAP_NUM_MAX;		//����J�n�Ԏ��ԍ�(ۯ����u��100����)
		end = LOCK_MAX;				//����I���Ԏ��ԍ�(ۯ����u��323�܂�)
	} else if (mtype == _MTYPE_FLAP) {
		start = CAR_START_INDEX;	//����J�n�Ԏ��ԍ�(�ׯ�ߑ��u��0����)
		end = CRR_CTRL_START;		//����I���Ԏ��ԍ�(IF�ׯ�ߑ��u��49�܂�)
	} else if (mtype == _MTYPE_INT_FLAP) {
		start = INT_CAR_START_INDEX;	//����J�n�Ԏ��ԍ�(�ׯ�ߑ��u��0����)
		end = FLAP_NUM_MAX;			//����I���Ԏ��ԍ�(�����ׯ�ߑ��u��99�܂�)
	} else if (mtype == _MTYPE_ALL) {
		start = CAR_START_INDEX;
		end = LOCK_MAX;
	} else {
		return 0;		// �����ɂ͗��Ȃ�
	}

	errnum = 0;

	if( OpenClose == 0 ){
		for( i = start; i < end; i++ ){
			WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( LockInfo[i].lok_syu ){
				if( FLAPDT.flp_data[i].nstat.bits.b04 ){
					errnum++;
				}
			}
		}
	}else{
		for( i = start; i < end; i++ ){
			WACDOG;												// ���u���[�v�̍ۂͳ����ޯ�ؾ�Ď��s
			if( LockInfo[i].lok_syu ){
				if( FLAPDT.flp_data[i].nstat.bits.b05 ){
					errnum++;
				}
			}
		}
	}
	return( (short)errnum );
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	�G���[���̃Z�b�g																						   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ErrBinDatSet()																			   |*/
/*| PARAMETER	: dat	: �װ���i�޲���ް��j																   |*/
/*|               knd	: 0:�ʐM�ُ� 1:�J��ۯ�																   |*/
/*| RETURN VALUE: 																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: T.Hashimo																					   |*/
/*| Date		: 2005-08-08																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	ErrBinDatSet( ulong dat, short knd )
{
	ulong	ul;

	if( knd == 0 ){
		ul = dat;
	}else{
		// �J��ۯ��̂Ƃ��͎Ԏ������
		ul = (ulong)LockInfo[dat].area * 10000;
		ul += (ulong)LockInfo[dat].posi;
	}
	Err_work.ErrBinDat = ul;
}

/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	���b�N�ُ�G���[����																					   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: lk_err_chk()																				   |*/
/*| PARAMETER	: lkno	: ۯ����u�ԍ�(0�`323)																   |*/
/*|               no	: �װ�ԍ�																			   |*/
/*|               knd	: 0:���� 1:����																		   |*/
/*| RETURN VALUE: 																							   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																					   |*/
/*| Date		: 2005-09-21																				   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
void	lk_err_chk( ushort lkno, char no, char kind )
{
	char	mtype;		// �װ�ԍ�(no)�͓���̒l�̂��߁A���̂܂܎g�p����

	if (lkno < FLAP_NUM_MAX) {
		// �ׯ�ߑ��u
		if( lkno < INT_CAR_START_INDEX )
			mtype = ERRMDL_IFFLAP;
		else
			mtype = ERRMDL_FLAP_CRB;
	} else {
		// ۯ����u
		mtype = ERRMDL_FLAP_CRB;
	}

	if( CPrmSS[S_TYP][70] ){

		// ��ϰ���Ԍ�(��ϰ���Դװ���ێ������)�װ�o�^
		if( kind == 0 ){	// ����
			ErrBinDatSet( (ulong)lkno, 1 );
			err_chk( mtype, no, kind, 2, 1 );
			if( no == ERR_LOCKCLOSEFAIL ){
				// ۯ����u�ُ�
				FLPCTL.Flp_uperr_tm[lkno] = -1;	// ��ϰ�į��
			}else{
				// ۯ����u�J�ُ�
				FLPCTL.Flp_dwerr_tm[lkno] = -1;	// ��ϰ�į��
			}
		}else{				// ����
			if( no == ERR_LOCKCLOSEFAIL ){
				// ۯ����u�ُ�
				FLPCTL.Flp_uperr_tm[lkno] = (short)(((( CPrmSS[S_TYP][70] / 100 ) * 60 ) + ( CPrmSS[S_TYP][70] % 100 )) / 5 );
			}else{
				// ۯ����u�J�ُ�
				FLPCTL.Flp_dwerr_tm[lkno] = (short)(((( CPrmSS[S_TYP][70] / 100 ) * 60 ) + ( CPrmSS[S_TYP][70] % 100 )) / 5 );
			}
		}
	}else{
		// �����װ�o�^
		ErrBinDatSet( (ulong)lkno, 1 );
		err_chk( mtype, no, kind, 2, 1 );
		FLPCTL.Flp_uperr_tm[lkno] = -1;		// ��ϰ�į��
		FLPCTL.Flp_dwerr_tm[lkno] = -1;		// ��ϰ�į��
	}
	/* �t���b�v���~���b�N�������ی쏈�� */
	if (lkno < FLAP_NUM_MAX && no == ERR_FLAPLOCK_LOCKOPENFAIL) {
		flp_DownLock_lk_err_chk( lkno, no, kind );
	}
}


// ۯ���������
/*[]----------------------------------------------------------------------[]
 *|	name	: get_lktype
 *[]----------------------------------------------------------------------[]
 *| summary	: ���b�N�����擾
 *| param	: lok_syu:���b�N���u���
 *| return	: �Y�����b�N����
 *[]----------------------------------------------------------------------[]*/
uchar get_lktype(uchar lok_syu)
{
	uchar type;
	
	if (lok_syu <= 6) {
		type = (uchar)prm_get( COM_PRM,S_TYP,60,1,(char)((6-lok_syu)+1) );
		if (type == 0) {
			type = (uchar)((lok_syu == 6) ? 3 : lok_syu);
		}
	}
	else {
		lok_syu -= 6;
		type = (uchar)prm_get( COM_PRM,S_TYP,61,1,(char)((6-lok_syu)+1) );
	}

	return type;
}

// �����A�����Z
#if (1 == AUTO_PAYMENT_PROGRAM)
/*[]----------------------------------------------------------------------[]*/
/*| �ׯ��/ۯ�����̎�M��а�֐�                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LK_AutoPayment_Rcv                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void LK_AutoPayment_Rcv(ushort index, uchar tno, uchar lkno, uchar kind)
{
	uchar  data[64];
	ushort size;
	uchar  carcnd, lkcnd;
	t_LKcomLock*		lck_tmp;
	t_LKcomSubLock*		lck_sub;
	t_IFM_mail	*pQbody;
	int	i;
	int imax;

	// �ް����
	switch (kind) {
	case AUTOPAY_STS_IN:		// �ԗ����m(����)
		carcnd = 1;
		lkcnd = 2;
		break;
	case AUTOPAY_STS_OUT:		// �ԗ����mOFF
		carcnd = 2;
		lkcnd = 2;
		break;
	case AUTOPAY_STS_UP:		// �ׯ��/ۯ�ON �ԗ��݂�
		carcnd = 1;
		lkcnd = 1;
		break;
	case AUTOPAY_STS_DOWN:		// �ׯ��/ۯ�OFF �ԗ��݂�
		carcnd = 1;
		lkcnd = 2;
		break;
	default:
		return;
	}

	memset(data, 0, sizeof(data));
	pQbody = (t_IFM_mail *)&data[0];						// ���̃o�b�t�@�ɂ�ID4����Z�b�g����B
	// �L���[���̂ցu�ʏ��ԃf�[�^�v���Z�b�g
	pQbody->s61.sCommon.ucKind           = LK_RCV_COND_ID;	// �ʏ��ԃf�[�^
	pQbody->s61.sCommon.bHold            = 1;				// �f�[�^�ێ��t���O
	pQbody->s61.sCommon.t_no             = tno;				// �[��No(CRB��)
	pQbody->s61.ucCount                  = 1;				// ��ԃf�[�^���B�����e�i���X�ʒm�̂��߂ɂ́u�P�v�ŏ\�� 
	if(LKcom_Search_Ifno(tno) == 0) {
		i = FLAP_NUM_MAX;									// ���b�N���u�C���f�b�N�X
		imax = LOCK_MAX;									// ���b�N���u�ő�C���f�b�N�X
	}
	else {
		i = CRR_CTRL_START;									// �t���b�v���u�C���f�b�N�X
		imax = TOTAL_CAR_LOCK_MAX;							// �t���b�v���u�ő�C���f�b�N�X
	}

	for( ; i<imax; ++i ){
		WACDOG;												// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LKcom_Search_Ifno(tno) == 0) {									// ���b�N�̏ꍇ
			if( (LockInfo[i].if_oya == tno)&&(LockInfo[i].lok_no == lkno)){// �^�[�~�i��No�ƃ��b�N���u�ԍ���v
				// LockInfo��LOCK_MAX�̒l��MAX�ł��邪�A255�Ԏ�������肻�̂܂܃Z�b�g
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfo�̃C���f�b�N�X���Z�b�g
				break;
			}
		} else {															// �t���b�v�̏ꍇ LockInfo[i].lok_no ���u1�v�Ɠǂݑւ���
			if( (LockInfo[i].if_oya == tno)&&(1 == lkno)){// �^�[�~�i��No�ƃ��b�N���u�ԍ���v
				// LockInfo��LOCK_MAX�̒l��MAX�ł��邪�A255�Ԏ�������肻�̂܂܃Z�b�g
				pQbody->s61.sLock[0].ucLock = (uchar)i;// LockInfo�̃C���f�b�N�X���Z�b�g
				break;
			}
		}
	}
	pQbody->s61.sLock[0].ucCarState  = carcnd;				// �ԗ����m���		=0�Œ�i�����e�i���X�J�n/�I����m�点��̂��ړI�j
	pQbody->s61.sLock[0].ucLockState = lkcnd;				// ���b�N���u���	(07H)�����e�i���X��/(08H)�����e�i���X�����̉��ꂩ 
	LKcom_RcvDataSave(&data[0],7);							// �A�v��������M����o�b�t�@�ɃZ�b�g
}

/*[]----------------------------------------------------------------------[]*/
/*| �ׯ��/ۯ��ւ̗v�����M������а�֐�                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LK_AutoPayment_Send                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void LK_AutoPayment_Send(uchar index, uchar tno, uchar lkno, uchar kind)
{
	switch (kind) {
	case 1:		// �ׯ�ߔ㏸
		// �㏸�����̉�����Ԃ�
		LK_AutoPayment_Rcv((ushort)index, tno, lkno, AUTOPAY_STS_UP);
		break;
	case 2:		// �ׯ�ߔ��~
		// ���~�����̉�����Ԃ�
		LK_AutoPayment_Rcv((ushort)index, tno, lkno, AUTOPAY_STS_DOWN);
		// �ԗ������̉�����Ԃ�
		LK_AutoPayment_Rcv((ushort)index, tno, lkno, AUTOPAY_STS_OUT);
		break;
	default :
		break;
	}
}

#endif

/*[]----------------------------------------------------------------------[]*/
/*| �^�[�~�i���ԍ�����CRA�ECRR����̓d�����𔻒f����                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_Search_Ifno	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka	                                           |*/
/*| Date         : 2009-06-05                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char	LKcom_Search_Ifno( uchar if_no )
{
	char	ret=0;
	if(LKCOM_TYPE_FLAP == LKcom_GetAccessTarminalType(if_no)) {
		ret = 1;
	}
		
	return ret;

}

/*[]----------------------------------------------------------------------[]*/
/*| �S�t���b�v�E���b�N���u�̏�Ԃ�v������			                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_AllInfoReq	                	                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Namioka	                            	               |*/
/*| Date         : 2009-10-04                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
void	LKcom_AllInfoReq( uchar kind )
{
	uchar type = GetCarInfoParam();
	
	switch( kind ){
		case _MTYPE_LOCK:
			if( !(type & 0x01) ){
				return;
			}
			break;
		case _MTYPE_INT_FLAP:
			if( !(type & 0x04) ){
				return;
			}
			break;
		case _MTYPE_FLAP:
			if( !(type & 0x02) ){
				return;
			}
			break;
		default:
			return;
	}
	queset( FLPTCBNO, LK_SND_A_STS, 1, &kind );		// �S��ԗv�����䑗�M�v��
}
/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��ԍ����瑕�u�̎�ʂ��擾����				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_RoomNoToType	               	                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2009-10-04                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
ushort	LKcom_RoomNoToType( ulong roomNo )
{
	ushort i;
	
	for( i=INT_CAR_START_INDEX; i < LOCK_MAX; ++i ){
		WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LockInfo[i].posi == roomNo) {
			if(i < BIKE_START_INDEX) {
				return 1;
			}
			else {
				return 0;
			}
		}
	}
	return (ushort)-1;
}
/*[]----------------------------------------------------------------------[]*/
/*| ���ʐݒ肩��ڑ�����[�������擾����	                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_GetAccessTarminalCount                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : �Ȃ�                                                          |*/
/*|	return : �[����                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2012-06-01                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	LKcom_GetAccessTarminalCount(void)
{
	long	terminalCount;								// �[����
	
	terminalCount = prm_get(COM_PRM, S_TYP, 41, 2, 1);
	if(terminalCount == 0) {							// ���ڐ���(FT-4000)��
		terminalCount = prm_get(COM_PRM, S_TYP, 42, 2, 5);
	}
	
	return (ushort)terminalCount;
}
/*[]----------------------------------------------------------------------[]*/
/*| �w�肵���[��No.�̑��u��ʂ��擾����	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_GetAccessTarminalType                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : tno	: �[��No.(1�`�j                                        |*/
/*|	return : ���u���                                                      |*/
/*|	           LKCOM_TYPE_NONE	: ���u�Ȃ�                                 |*/
/*|	           LKCOM_TYPE_FLAP	: ���u�̓t���b�v                           |*/
/*|	           LKCOM_TYPE_LOCK	: ���u�̓��b�N                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2012-06-01                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	LKcom_InitAccessTarminalType(void)
{
	ushort	terminalMax;								// �[����
	ushort	terminal;									// �[���J�E���^
	
	memset(LKcom_Type, 0, sizeof(LKcom_Type));
	
	terminalMax = LKcom_GetAccessTarminalCount();
	if(terminalMax > LOCK_IF_MAX){
		terminalMax = LOCK_IF_MAX;// LKcom_Type�̃I�[�o�[���C�g�}�~
	}
	for(terminal = 0; terminal < terminalMax; ++terminal) {
		LKcom_Type[terminal] = LKcom_GetAccessTarminalType(terminal + 1);
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| �w�肵���[��No.�̑��u��ʂ��擾����	                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_GetAccessTarminalType                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : tno	: �[��No.(1�`�j                                        |*/
/*|	return : ���u���                                                      |*/
/*|	           LKCOM_TYPE_NONE	: ���u�Ȃ�                                 |*/
/*|	           LKCOM_TYPE_FLAP	: ���u�̓t���b�v                           |*/
/*|	           LKCOM_TYPE_LOCK	: ���u�̓��b�N                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2012-06-01                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
uchar	LKcom_GetAccessTarminalType(ushort tno)
{
	ushort	index;
	uchar	work;
	ushort	startIndex;
	ushort	endIndex;
	
	if(tno == 0) {
		return LKCOM_TYPE_NONE;
	}
	
	work = GetCarInfoParam();						// ���b�N�^�t���b�v���u����ݒ�擾
	switch(work){
		case 0x01 : 								// ���b�N���u�̂ݐݒ肠��̏ꍇ
			startIndex = BIKE_START_INDEX;
			endIndex = LOCK_MAX;
			break;
		case 0x04 : 								// �t���b�v���u�̂ݐݒ肠��̏ꍇ
			startIndex = INT_CAR_START_INDEX;
			endIndex = BIKE_START_INDEX;
			break;
		case 0x05 : 								// �t���b�v�ƃ��b�N���ɐݒ肠��̏ꍇ
			startIndex = INT_CAR_START_INDEX;
			endIndex = LOCK_MAX;
			break;
		default :									// ���ݒ�̏ꍇ
			startIndex = 0;
			endIndex = 0;
			break;
	}
	for(index = startIndex; index < endIndex; ++index ) {
		WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LockInfo[index].if_oya == tno) {
			if(index < BIKE_START_INDEX) {
				return LKCOM_TYPE_FLAP;				// ���u�̓t���b�v�i���ԁj
			}
			else {
				return LKCOM_TYPE_LOCK;				// ���u�̓��b�N�i���ցj
			}
		}
	}
	return	LKCOM_TYPE_NONE;
}
// MH322914(S) K.Onodera 2016/09/07 AI-V�Ή��F�G���[�A���[��
/*[]----------------------------------------------------------------------[]*/
/*| �Ԏ��ԍ�����C���f�b�N�X���擾����				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LKcom_RoomNoToType	               	                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : 			                            	               |*/
/*| Date         : 2009-10-04                                              |*/
/*[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]*/
int	LKcom_RoomNoToIndex( ulong roomNo )
{
	int		i;
	uchar	hit = 0;
	
	for( i=INT_CAR_START_INDEX; i < LOCK_MAX; ++i ){
		WACDOG;										// ���u���[�v�̍ۂ�WDG���Z�b�g���s
		if(LockInfo[i].posi == roomNo) {
			hit = 1;
			break;
		}
	}
	if( !hit ){
		i = -1;
	}
	return i;
}
// MH322914(E) K.Onodera 2016/09/07 AI-V�Ή��F�G���[�A���[��