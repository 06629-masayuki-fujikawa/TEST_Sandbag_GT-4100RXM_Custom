/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		IF�ռرْʐM�֘A�ϐ�										  										   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      :  K.Akiba																					   |*/
/*| Date        :  2005-05-06																				   |*/
/*| Update      :																							   |*/
/*[]---------------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
																				/*								*/
#include	"iodefine.h"														/*								*/
#include	"system.h"															/*								*/
#include	"LKcom.h"															/*								*/
/*--------------*/																/*								*/
/*	LKcom.c		*/																/*								*/
/*--------------*/																/*								*/
uchar			LKcom_InitFlg;													/* ۯ����u�ʐM�������׸�		*/
																				/*								*/
t_LKcomLineCtrl	LKcomLineCtrl;													/* ������۰�					*/
																				/*								*/
t_LK_RCV_INFO	LKcom_RcvData;													/*								*/
																				/*								*/
																				/*								*/
uchar			LKcom_f_SndReq;													/* POL/SEL���Mظ���				*/
uchar			LKcom_f_TaskStop;												/* ۯ����u�ʐM�����~�׸�		*/
																				/*								*/
																				/*								*/
/*--------------*/																/*								*/
/*	LKcomdr.c	*/																/*								*/
/*--------------*/																/*								*/
uchar			LKcomdr_RcvData[1024];											/*								*/
																				/*								*/
																				/*								*/
/*--------------*/																/*								*/
/*	LKcomApi.c	*/																/*								*/
/*--------------*/																/*								*/
uchar			LKcom_RcvDtWork[1024];											/* ��M�ް���͗pܰ�			*/
																				/*								*/
uchar			MntLockTest[LK_LOCK_MAX];										/* ýČ���						*/
uchar			MntFlapTest[CAR_LOCK_MAX];										/* ýČ��ʊi�[�ر(���g�p)		*/

ulong			MntLockDoCount[BIKE_LOCK_MAX][3];								/* LOCK���u���춳��				*/
ulong			MntFlapDoCount[TOTAL_CAR_LOCK_MAX][3];							/* �ׯ�ߑ��u���춳��			*/
ushort			MntLockWkTbl[LOCK_IF_REN_MAX];									/* ۯ����u���춳�Ď�M�p�ϊ�ð���	*/
																				/*								*/
t_LKBUF			LockBuf[LK_LOCK_MAX];											/* ����ް���M�ꎞ�ޔ��ޯ̧	*/
																				/*								*/
uchar			child_mk[LOCK_IF_MAX];											// �q�@��ۯ����uҰ���ҏW
																				/*								*/
																				/*								*/
uchar			flp_m_mode[FLAP_IF_MAX];										/* �ׯ�ߑ��u�蓮Ӱ���׸�		*/
																				/*  ON = �蓮Ӱ�ޒ�				*/
uchar			lok_m_mode[LOCK_IF_MAX];										/* ۯ����u�蓮Ӱ���׸�			*/
																				/*  ON = �蓮Ӱ�ޒ�				*/
uchar			LKcom_Type[LOCK_IF_MAX];										// �[�����̑��u��ʁi���ԁE���ցj
																				/*------------------------------*/
