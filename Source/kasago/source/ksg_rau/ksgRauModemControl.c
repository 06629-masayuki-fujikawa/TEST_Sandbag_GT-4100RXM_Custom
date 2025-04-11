/************************************************************************************************/
/*																								*/
/*	Ӽޭ�ٖ���	:ksgRauModemControl.c	:-----------------------------------------------------: */
/*	Ӽޭ�ټ����	:						:���ѐ���											  : */
/*	���߲�		:						:													  : */
/*	�ް�ޮ�		:						:													  : */
/*	���ޯ�CPU	:						:-----------------------------------------------------: */
/*	�Ή��@��	:						: 													  : */
/*										:-----------------------------------------------------: */
/*																								*/
/************************************************************************************************/

#include "system.h"														/*							*/
																		/*							*/
#include 	<machine.h>													/*							*/
#include 	<string.h>													/*							*/
#include 	"iodefine.h"												/* I/O��`					*/
#include	"message.h"
#include 	"ksgmac.h"													/* MAC ���ڋ@�\ͯ��			*/
#include 	"ksgRauTable.h"												/* ð����ް���`			*/
#include 	"ksgRauModem.h"												/* ���ъ֘A�֐��S			*/
#include 	"ksgRauModemData.h"											/* ���ъ֘A�ް���`			*/
#include	"ksg_def.h"
#include	"raudef.h"
// GG109800(S) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
#include	"prm_tbl.h"	
// GG109800(E) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
																		/*							*/
BITS	mdm_result_f;													/* ػ��ĺ��ޥ�׸�			*/
#define f_OK		mdm_result_f.BIT.B0									/* OK						*/
#define f_CONN		mdm_result_f.BIT.B1									/* CONNECT					*/
#define f_NOCON		mdm_result_f.BIT.B2									/* NO CARRIER				*/
#define f_BUSY		mdm_result_f.BIT.B3									/* BUSY						*/
#define f_DELAYED	mdm_result_f.BIT.B4									/* DELAYED					*/
#define f_ERR		mdm_result_f.BIT.B5									/* ERROR					*/
#define f_RING	 	mdm_result_f.BIT.B6									/* RING						*/
#define f_ECHOBACK 	mdm_result_f.BIT.B7									/* ATE0						*/
																		/*							*/
BITS	mdm_init_f;
#define	f_TYPE_OK	mdm_init_f.BIT.B0									// �@��ڑ�OK

extern	unsigned short	KSG_Tm_Reset_t;									/* �@��ؾ�ďo�����			*/
extern	unsigned short	PC_STEP;										/* 							*/
static	unsigned char	KSG_NoReciveCnt;								/* ����������				*/

BITS	KSG_uc_mdm_res_Foma_f;
BITS	KSG_uc_mdm_ErrSndReq_f;

#define	f_resFomaRestriction 	KSG_uc_mdm_res_Foma_f.BIT.B0			/* RESTRICTION				*/
#define f_resFomaAntenaLow	 	KSG_uc_mdm_res_Foma_f.BIT.B1			/* *DANTE:0					*/
#define f_ErrSndReqERRCD		KSG_uc_mdm_ErrSndReq_f.BIT.B0			/* MODEM ERROR CD HIGH		*/
#define	f_ErrSndReqMDMOFF		KSG_uc_mdm_ErrSndReq_f.BIT.B1			/* MODEM POWER OFF Request.	*/

unsigned long  ConnTimeOutTimer;

//extern signed long	DownLoadOffDelay;									/* �޳�۰�ގ��̐ؒf�܂ł��ިڲ 100m x n */
//extern signed long	KSG_DownLoadSafeTime;								/* ����޲��ѵ��ް������ϰ */
																		/*							*/
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
extern uchar	KSG_RauBkupAPNName[32];// APN�̐ݒ�l�̔�r�p�o�b�N�A�b�v FOMA���W���[���ɐݒ�ς݂̃f�[�^����r���邽�߂̂���
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������

enum FOMA_AT_FHASE {
	FOMA_AT_PHASE_PURA3 = 1,	// +++
	FOMA_AT_PHASE_ATH0,			// ATH0
	FOMA_AT_PHASE_GMM,			// AT+GMM
	FOMA_AT_PHASE_ATZ,			// ATZ
	FOMA_AT_PHASE_ATS0,			// ATS0=1
	FOMA_AT_PHASE_ATE0,			// ATE0
	FOMA_AT_PHASE_CNUM,			// AT+CNUM
	FOMA_AT_PHASE_DRPW,			// AT*DRPW
	FOMA_AT_PHASE_DANTE,		// AT*DANTE
	FOMA_AT_PHASE_CGDCONT,		// AT+CGDCONT=
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
//	FOMA_AT_PHASE_MAX			// FOMA AT����������ސ�
	FOMA_AT_PHASE_KPPPAUTH,		// AT*KPPPAUTH UM04�Ή� PPP�F�ؕ����ݒ�
	FOMA_AT_PHASE_KALWAYSON,	// AT*KALWAYSON UM04�Ή� �펞�ڑ��ݒ�
	FOMA_AT_PHASE_MAX			// FOMA AT����������ސ�
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
};

char * const mdmFoMa_cmd[] = {
	"",					//dumy//
	"+++",				//1//
	"ATH0\r\n",			//2//
	"AT+GMM\r\n",		//3//
	"ATZ\r\n",			//4//
	"ATS0=0\r\n",		//5//
	"ATE0\r\n",			//6//
	"AT+CNUM\r\n",		//7//
	"AT*DRPW\r\n",		//8//
	"AT*DANTE\r\n",		//9//
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
//	""					//10// AT+CGDCONT
	"",					//10// AT+CGDCONT
	"AT*KPPPAUTH=1,0\r\n",//11// UM04�Ή� PPP�F�ؕ����ݒ�
	"AT*KALWAYSON=1,3,0\r\n"//12// UM04�Ή� �펞�ڑ��ݒ�
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
};
// GG109800(S) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
char * const mdmFoMa_cmd2[] = {
	"",					//dumy//
	"+++",				//1//
	"ATH0\r\n",			//2//
	"AT+GMM\r\n",		//3//
	"ATZ\r\n",			//4//
	"ATS0=0\r\n",		//5//
	"ATE0\r\n",			//6//
	"AT+CNUM\r\n",		//7//
	"AT*DRPW\r\n",		//8//
	"AT*DANTE\r\n",		//9//
	"",					//10// AT+CGDCONT
	"AT*KPPPAUTH=1,2\r\n",//11// UM04�Ή� PPP�F�ؕ����ݒ�  �F�ؕ���CHAP
	"AT*KALWAYSON=1,3,2,\"amano\",\"amano\"\r\n"//12// UM04�Ή� �펞�ڑ��ݒ� �F�ؕ���CHAP ID amano PASS amano
};
// GG109800(E) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
const char KSG_RauMdmFmaATCGD_1[]={"AT+CGDCONT=1,\"PPP\",\""};	// Access Point Name Setting

#define	ANTENA_REQCNT_MAX		60
enum Antena_Level {
	LEVEL_0 = 0,	// �A���e�i���x��0
	LEVEL_1,		// �A���e�i���x��1
	LEVEL_2,		// �A���e�i���x��2
	LEVEL_MAX		// �A���e�i���x��3
};
static enum Antena_Level antena_level = LEVEL_MAX;
static unsigned short antena_req_cnt = 0;		// FOMA�A���e�i���x���v����������
static unsigned short antena_req_interval = 0;	// FOMA�A���e�i���x���v���Ԋu
int		KSG_gAntLevel;							// �A���e�i���x��
int		KSG_gReceptionLevel;					// ��M�d�͎w�W
unsigned char		KSG_AntennaLevelCheck = 0;		// �A���e�i���x���`�F�b�N��� 0:���v�� 1:�v����
unsigned char		KSG_AntAfterPppStarted = 0;		// �A���e�i���x���`�F�b�N�ɂ��E7778�}�~�p
unsigned short		reception_level = 0;

#define		KSG_PPP_CLOSE_WAIT_TIME		30 * 100	// �A���e�i���x���`�F�b�N����PPP�ؒf�҂�����(10ms�P��)

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���Ѻ��۰�																				   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmControl();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmControl(void)
{
	unsigned char	uc_ATinitphaseMax;

	uc_ATinitphaseMax = 7;							//DOPA

	if ( KSG_uc_FomaFlag == 1 ){
		// AT����ސ�(+1)
		uc_ATinitphaseMax = FOMA_AT_PHASE_MAX + 1;	//FOMA

		if ( KSG_mdm_status == 1 ){
			// ER HIGH��� DR HIGH�Ď�.//
			if( (KSG_line_stat_h & KSG_DR_SIG) == 0 ){	// ���f�����c�q OFF
				if ( KSG_uc_FomaWaitDRFlag == 0 ){
					KSG_uc_FomaWaitDRFlag = 1;		// �Ď��� //
					KSG_f_ERDRtime_rq = KSG_CLR;
					KSG_Tm_ERDR_TIME.BIT.bit0 = (ushort)KSG_Tm_FomaMdmDRwait; // 5�b
					if (KSG_uc_AdapterType == 1) {
					// UM03-KO�͂��������N�����Ԃ��|����
						KSG_Tm_ERDR_TIME.BIT.bit0 = (unsigned short)-UM03KO_ER_DR_Int;
					}
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
					else if (KSG_uc_AdapterType == 2){
						// UM04-KO�p�̋N�����Ԑݒ�
						KSG_Tm_ERDR_TIME.BIT.bit0 = (unsigned short)-UM04KO_ER_DR_Int;
					}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
					KSG_f_ERDRtime_ov = KSG_CLR;
					KSG_f_ERDRtime_rq = KSG_SET;
				}
				else if ( KSG_f_ERDRtime_ov == KSG_SET ){
					KSG_f_ERDRtime_ov = KSG_CLR;
					RauSysErrSet( RAU_ERR_FMA_MODEMERR_DR, 1 );
					if ( KSG_uc_MdmInitRetry < 3 ){
						KSG_uc_MdmInitRetry++;
						KSG_uc_FomaWaitDRFlag = 0;
						KSG_RauModem_OFF_2( );
					}
				}
			}else{
				KSG_f_ERDRtime_ov = KSG_CLR;
				KSG_f_ERDRtime_rq = KSG_CLR;
				KSG_uc_FomaWaitDRFlag = 0;
				RauSysErrSet( RAU_ERR_FMA_MODEMERR_DR, 0 );
				RauSysErrSet( RAU_ERR_FMA_MODEMERR_CD, 0 );
				KSG_uc_MdmInitRetry = 0;
			}
		}
		else if (KSG_mdm_status > 1 ){
			KSG_f_ERDRtime_ov = KSG_CLR;
			KSG_f_ERDRtime_rq = KSG_CLR;
			KSG_uc_FomaWaitDRFlag = 0;
			RauSysErrSet( RAU_ERR_FMA_MODEMERR_DR, 0 );
			RauSysErrSet( RAU_ERR_FMA_MODEMERR_CD, 0 );
			KSG_uc_MdmInitRetry = 0;
		}

		if( f_ErrSndReqMDMOFF == 1 ){
			f_ErrSndReqMDMOFF = 0;
 			RauSysErrSet( RAU_ERR_FMA_MODEMPOWOFF, 0 );
		}
		if( f_ErrSndReqERRCD == 1 ){
			f_ErrSndReqERRCD = 0;
			RauSysErrSet( RAU_ERR_FMA_MODEMERR_CD, 1 );
		}

	}
	switch( KSG_mdm_status ) {											/* ���ѽð���ɂ�镪��		*/
	/* --- DTE Ready ---------------------------------------------------------------------------*/
	case 1:																/*							*/
		SCI7_DTR = 0;													/* DTR ON					*/
		if( KSG_line_stat_h & KSG_DR_SIG ) {							/* DSR ON?					*/
			KSG_mdm_status = 2;											/* ��DTE-DCE�m��			*/
			KSG_NoReciveCnt = 0;										/* �����������ر			*/
			KSG_f_init_repeat = 0;	f_OK = 1;							/* ��������ؾ��			*/
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;							/* ��ϒ�~					*/
			KSG_Tm_AtCom.BIT.bit0 = (ushort)KSG_Tm_AtCom_3s;			/* ��3�b��Ͼ��				*/
			KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;	KSG_Tm_AtCom.BIT.f_rq = KSG_SET;	/* ��Ͻ���					*/
			SCI7_RTS = 0; 												/* RTS ON					*/
			mdm_init_f.BYTE = 0;
		}																/*							*/
		break;															/*							*/

	/* --- DTE-DCE�m�� -------------------------------------------------------------------------*/
	case 2:																/*							*/
		KSG_f_DialConn_ov = KSG_CLR;	KSG_f_DialConn_rq = KSG_CLR;	/* ����ڑ��҂���ϒ�~		*/
		if( ( KSG_line_stat_h & KSG_DR_SIG ) == 0) {					/* DSR OFF?					*/
			KSG_mdm_status = 1;											/* ��DTE Ready				*/
			KSG_mdm_init_sts = 1;										/* ���я������v�� 			*/
			SCI7_RTS = 1; 												/* RTS OFF					*/
		}																/*							*/
		else if( KSG_mdm_init_sts < uc_ATinitphaseMax ) {				/* DSR ON? <���я������҂�>	*/
			SCI7_RTS = 0; 												/* RTS ON					*/
			if( KSG_f_Gurd_ov == 1 ) {									/* �޲�ٶް�����			*/
				if ( KSG_uc_FomaFlag == 1 ) {
				//==================================================================================
				//	FOMA����������ޔ��s����
				//
				//  OK ������3�b�ԑ҂A3�b�ȓ��� OK ��M���� 1�b���Ă��A���̺���ނ𑗐M����
				//  3�b�o�߂��Ă� OK �����������ꍇ����ײ���A3����ײ���s�� Modem Reset������Ȃ���.
				//==================================================================================
					if ( KSG_uc_AtResultWait == 0 ){
						//** ����ޑ��M **//
						if (KSG_mdm_init_sts < FOMA_AT_PHASE_MAX) {
							// ��ϵ��ް����(�O�̺���ނ���̳���)
							if( KSG_Tm_AtCom.BIT.f_ov == 1 ) {
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
								if( KSG_uc_AdapterType == 2){// UM04-KO
									if (KSG_mdm_init_sts == FOMA_AT_PHASE_CGDCONT) {
										if(0 == memcmp(KSG_RauBkupAPNName,KSG_RauConf.APNName,sizeof(KSG_RauBkupAPNName))) {
											// APN�����W���[���ɐݒ�ς݂̏ꍇ�A���f���������I���Ƃ���
											KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
											KSG_Tm_AtCom.BIT.bit0 = (ushort)-20;			// 2s timer set
											KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
											KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
											KSG_f_mdm_init_err = 0;
											mdm_result_f.BYTE = 0;
											KSG_uc_mdm_res_Foma_f.BYTE = 0;
											KSG_f_init_repeat = 0;
											KSG_mdm_init_sts = FOMA_AT_PHASE_MAX + 1;	// ���f������������ //
											break;
										}
										
									}
								}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
								KSG_uc_AtResultWait = 1;					// �����҂���� //
								//	FOMA�����������
								mdm_result_f.BYTE = 0;
								KSG_uc_mdm_res_Foma_f.BYTE = 0;
																		// AT ����ޑ��M�v�� //
								KSG_RauMdmInitCmdSetFOMA( KSG_mdm_init_sts );

								//	��ϒl�ݒ�
								KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
								if ( KSG_mdm_init_sts == FOMA_AT_PHASE_PURA3 ){		// +++ �K�� OK ������킯�ł͂Ȃ�,�����ߐ�����OK,�����ߒ��͖����� //
									f_OK = 1;
								}
								if ( KSG_mdm_init_sts == FOMA_AT_PHASE_ATH0 ){
									KSG_Tm_AtCom.BIT.bit0 = (ushort)-300;	// 30s ATH0 �� CD������̂�҂ꍇ���邽�ߒ���.. //
								}else{
									KSG_Tm_AtCom.BIT.bit0 = (ushort)-30;	// 3s �����҂����� //
								}
								KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
								KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							}
						}
						else {
							KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
							KSG_Tm_AtCom.BIT.bit0 = (ushort)-20;			// 2s timer set
							KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
							KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							KSG_f_mdm_init_err = 0;
							mdm_result_f.BYTE = 0;
							KSG_uc_mdm_res_Foma_f.BYTE = 0;
							KSG_f_init_repeat = 0;
							KSG_mdm_init_sts = FOMA_AT_PHASE_MAX + 1;	// ���f������������ //
						}
					}else{
						//** �����҂� **//
						if ( f_OK == 1 ){							// OK ��M //

// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
							if( KSG_uc_AdapterType == 2){// UM04-KO
								if (KSG_mdm_init_sts == (FOMA_AT_PHASE_MAX - 1)) {// �R�}���h�Ō�܂ŏI��
									// UM04-KO�̏ꍇ�A�ڑ���ݒ�uAT+CGDCONT�v�Ə펞�ڑ��ݒ�uAT*KALWAYSON�v�����s��̓��f�������u�[�g����K�v������
									memcpy ( &KSG_RauBkupAPNName, &KSG_RauConf.APNName, sizeof(KSG_RauBkupAPNName) );// APN�̐ݒ肪���������̂Ńo�b�N�A�b�v
									KSG_RauModem_OFF();
								}
							}else{
								// UM03-KO�ȑO�́uAT+CGDCONT=�v���Ō�̃R�}���h
								if (KSG_mdm_init_sts == (FOMA_AT_PHASE_CGDCONT)) {// �R�}���h�Ō�܂ŏI��
									KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;
								}
							}

// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
							if ( f_resFomaAntenaLow	== 1 ){
							/* �A���e�i���x���G���[�͔����������� */
								switch(antena_level){
								case LEVEL_0:
									RauSysErrSet( RAU_ERR_FMA_ANTENALOW, 0 );
									break;
								case LEVEL_1:
									RauSysErrSet( RAU_ERR_FMA_ANTENALOW1, 0 );
									break;
								case LEVEL_2:
									RauSysErrSet( RAU_ERR_FMA_ANTENALOW2, 0 );
									break;
								default:
									break;
								}
							}

							if (KSG_mdm_init_sts == FOMA_AT_PHASE_GMM) {
								if (f_TYPE_OK) {
									RauSysErrSet( RAU_ERR_FMA_ADAPTER_NG, 0 );		// FOMA�A�_�v�^�ݒ�s�� ����
								}
								else {
									RauSysErrSet( RAU_ERR_FMA_ADAPTER_NG, 1 );		// FOMA�A�_�v�^�ݒ�s�� ����
								}
							}

							KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
							KSG_Tm_AtCom.BIT.bit0 = (ushort)-12;		// 1s ���� AT ���M�܂ł̃E�F�C�g 1S�ۏ�,�덷100m +��.
							KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
							KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							KSG_uc_AtResultWait = 0;
							KSG_uc_AtRetryCnt = 0;
							KSG_mdm_init_sts ++;						// ���� AT �R�}���h�� .

						}else if ( f_NOCON == 1 && KSG_mdm_init_sts == FOMA_AT_PHASE_ATH0 ){ // ATH0 --> NO CARRIER //
							KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
							KSG_Tm_AtCom.BIT.bit0 = (ushort)-12;		// 1s ���� AT ���M�܂ł̃E�F�C�g 1S�ۏ�,�덷100m +��.
							KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
							KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
							KSG_uc_AtResultWait = 0;
							KSG_uc_AtRetryCnt = 0;
							KSG_mdm_init_sts ++;						// ���� AT �R�}���h�� .
							RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 1 );	// NO CARRIER����

						}else if ( KSG_Tm_AtCom.BIT.f_ov == KSG_SET ){// �����҂� Time Out //

							KSG_uc_AtResultWait = 0;

							if ( (++KSG_uc_AtRetryCnt) >= 3 ){
																	// �������� 3��A��. //
								KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
								KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
								if ( f_ERR == 1 && KSG_mdm_init_sts == FOMA_AT_PHASE_CNUM ){ // AT+CNUM --> ERROR //
									RauSysErrSet( RAU_ERR_FMA_SIMERROR, 1 );	// SIM�J�[�h�O�ꌟ�m�G���[����
								}
								KSG_f_mdm_init_err = 1;
								f_ERR = 0;
								KSG_uc_AtRetryCnt = 0;
								KSG_RauModem_OFF();
							}else{
								KSG_Tm_AtCom.BIT.f_ov = KSG_SET;		// Retry.
							}
						}
					}
					break;		// BREAK ! //
				}
			}															/*							*/
		}																/*							*/
		//=======================================================================================
		// AT ��������.  ���M�ҋ@
		//=======================================================================================
		else if( 	( KSG_f_mdm_init_err == 0 ) &&						/*							*/
					( KSG_Tm_AtCom.BIT.f_ov == KSG_SET ) ) {			/* ���я������ς�?			*/
			SCI7_RTS = 0; 												/* RTS ON					*/
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;							/* ��ϒ�~					*/
			if( ( f_NOCON == 1 ) && ( KSG_mdm_mode == 0 ) ) {			/* "NO CARRIER"				*/
				f_NOCON = 0;											/*							*/
				RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 1 );				/* NO CARRIER����			*/
			}															/*							*/
			if( KSG_f_Gurd_ov == 1 ) {									/* �ް����ϒ�~��?			*/
				if( KSG_line_stat_h & KSG_CD_SIG ) {					/* ��CD ON?					*/
					f_CONN = 0;											/*							*/
					KSG_mdm_status = 4;									/* �ʐM��(���M)				*/
// MH322914 (s) kasiyama 2016/07/13 �Ĕ��đ҂��^�C�}�������������Ă��Ȃ��o�O�ɑΉ�����[���ʃo�ONo.1148](MH341106)
					KSG_mdm_cut_req = 0;								/* modem�ؒf���N�G�X�g�t���OOFF */
// MH322914 (e) kasiyama 2016/07/13 �Ĕ��đ҂��^�C�}�������������Ă��Ȃ��o�O�ɑΉ�����[���ʃo�ONo.1148](MH341106)
					KSG_dials.BIT.in = 1;								/* ���M						*/
				}														/*							*/
				else {													/* ��νĐڑ��v���׸�?		*/
					KSG_RauMdmConnect();								/* ��νĐڑ��v��			*/
					KSG_dials.BIT.ing = 1;								/* �޲�ْ�					*/
					KSG_f_host_conn_req = 0;							/*							*/
					ConnTimeOutTimer = KSG_Now_t;
					if( ( KSG_RauConf.Dpa_cnct_rty_cn > 0 )&&			/* ���_�C�A����			*/
						( KSG_RauConf.Dpa_cnct_rty_cn < 999 ) ) {		/*							*/
						KSG_Cnt_Retry = KSG_RauConf.Dpa_cnct_rty_cn;	/* �ڑ���ײ���� ���			*/
					}													/*							*/
					else {												/*							*/
						KSG_Cnt_Retry = 999;							/* �ڑ���ײ���� ���g�p		*/
					}													/*							*/

					KSG_Tm_ReDialTime.BIT.f_rq = KSG_CLR;				/* ��ϒ�~					*/
					KSG_Tm_ReDialTime.BIT.bit0 = (ushort)-180;			// 3�� //
					KSG_Tm_ReDialTime.BIT.f_ov = KSG_CLR;				/* ���ް�۰����				*/
					KSG_Tm_ReDialTime.BIT.f_rq = KSG_SET;				/* ��ϊJ�n					*/

					KSG_mdm_status = 3;									/* �ڑ���(���M)				*/
					KSG_uc_ConnAftWait = 0;
					RauSysErrSet( RAU_ERR_FMA_SIMERROR, 0 );			/* SIM�J�[�h�O�ꌟ�m�G���[���� */
				}														/*							*/
			}															/*							*/
		}																/*							*/
		break;															/*							*/

	/* --- �ڑ��� ------------------------------------------------------------------------------*/
	case 3:																/*							*/
		if( KSG_line_stat_h & KSG_CD_SIG ) {							/* CD ON?					*/
			if ( KSG_uc_FomaFlag == 1 ){
				if ( KSG_uc_ConnAftWait == 0 )
				{
					// RESTRICTION ����
					RauSysErrSet( RAU_ERR_FMA_RESTRICTION, 0 );

					KSG_uc_ConnAftWait = 1;
					KSG_Tm_AtCom.BIT.bit0 = (ushort)-6;					// 500msec+N
					KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
					KSG_Tm_AtCom.BIT.f_rq = KSG_SET;

					break;
				}else{
					if ( KSG_Tm_AtCom.BIT.f_ov != KSG_SET ){
						break;
					}
					KSG_uc_ConnAftWait = 0;								// CONNECT�㳴�ĊǗ��׸� //
				}
			}
			if( f_CONN == 1 ) {
				RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 0 );				/* NO CARRIER����			*/
			}
			f_CONN = 0;													/*							*/
			KSG_NoReciveCnt = 0;										/* �����������ر			*/
			KSG_mdm_status = 4;											/* �ʐM��(���M)				*/
			KSG_f_host_conn_req = 0;									/* νĐڑ��v���׸�			*/
			if( KSG_dials.BIT.ing ) {
				KSG_dials.BYTE = 1;										/* ���M						*/
			}
			else {
				KSG_dials.BYTE = 2;										/* ���M						*/
			}
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* ����ڑ��҂���ϒ�~		*/
// MH322914 (s) kasiyama 2016/07/13 �Ĕ��đ҂��^�C�}�������������Ă��Ȃ��o�O�ɑΉ�����[���ʃo�ONo.1148](MH341106)
			KSG_mdm_cut_req = 0;										/* modem�ؒf���N�G�X�g�t���OOFF */
// MH322914 (e) kasiyama 2016/07/13 �Ĕ��đ҂��^�C�}�������������Ă��Ȃ��o�O�ɑΉ�����[���ʃo�ONo.1148](MH341106)
			// PPP�ڑ��s�ǉ���
			RauSysErrSet( RAU_ERR_FMA_PPP_TIMEOUT, 0 );
			break;														/*							*/
		}																/*							*/
		else if( ( f_BUSY == 1 ) ||										/* "BUSY"					*/
		       ( ( f_NOCON == 1 ) && ( KSG_mdm_mode == 0 ) ) ) {		/* "NO CARRIER"				*/
			if( f_BUSY ) {												/*							*/
				f_BUSY = 0;												/*							*/
			}															/*							*/
			if( f_NOCON ) {												/*							*/
				f_NOCON = 0;											/*							*/
				RauSysErrSet( RAU_ERR_FMA_NOCARRIER, 1 );				/* NO CARRIER����			*/
			}															/*							*/
			KSG_NoReciveCnt = 0;										/* �����������ر			*/
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* ����ڑ��҂���ϒ�~		*/
			KSG_dials.BYTE = 0;											/* �޲�ٽð��ؾ��			*/
			if( KSG_Cnt_Retry > 0 ) {									/* ��ײ������?			*/
				KSG_f_host_conn_req = 1;								/* ��νĐڑ��v���׸�		*/
				if( KSG_RauConf.Dpa_cnct_rty_tm > 0 ) {					/* 							*/
					KSG_Tm_ReDial.BIT.bit0 = -KSG_RauConf.Dpa_cnct_rty_tm;	/* ��޲�يԊu��ϒl���		*/
					KSG_f_ReDial_rq = KSG_SET;		KSG_f_ReDial_ov = KSG_CLR;		/* ��޲�يԊu��ϊJ�n		*/
				}														/*							*/
				else {													/*							*/
					KSG_Tm_ReDial.BIT.bit0 = 0;							/* ��޲�يԊu��ϒl���		*/
					KSG_f_ReDial_rq = KSG_CLR;		KSG_f_ReDial_ov = KSG_SET;		/* ��޲�يԊu��ϊJ�n		*/
				}														/*							*/
				if( ( KSG_RauConf.Dpa_cnct_rty_cn > 0 )&&				/* ���_�C�A����			*/
						( KSG_RauConf.Dpa_cnct_rty_cn < 999 ) ) {		/*							*/
					KSG_Cnt_Retry--;									/* �ڑ���ײ���� ���Z		*/
				}
			}															/*							*/
			else {														/*							*/
				KSG_f_mdm_conn_err = 1;									/* νĐڑ��װ				*/
				KSG_mdm_init_sts = 0;									/* ���я����������~		*/
				if ( KSG_uc_FomaFlag == 1 ){
					// ���_�C�A���񐔃I�[�o�[
					RauSysErrSet( RAU_ERR_FMA_REDIALOV, 0 );
					if ( f_resFomaRestriction == 1 ){
						f_resFomaRestriction  = 0;
						// RESTRICTION ����
						RauSysErrSet( RAU_ERR_FMA_RESTRICTION, 1 );
					}
				}
				KSG_RauModem_OFF();										/*							*/
			}															/*							*/
		}																/*							*/
		else if( f_DELAYED == 1 ) {										/*							*/
			KSG_NoReciveCnt = 0;										/* �����������ر			*/
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* ����ڑ��҂���ϒ�~		*/
			f_DELAYED = 0;												/*							*/
			KSG_dials.BIT.ing = 0;										/* �޲�ق��ĂȂ�			*/
			KSG_f_host_conn_req = 1;									/* νĐڑ��v���׸�			*/
			KSG_Tm_ReDial.BIT.bit0 = (ushort)-180;						/* ��޲�يԊu��ϒl���		*/
			KSG_f_ReDial_rq = KSG_SET;		KSG_f_ReDial_ov = KSG_CLR;	/* ��޲�يԊu��ϊJ�n		*/
		}																/*							*/
		else if(( KSG_f_ReDial_ov == 1 ) && ( KSG_f_host_conn_req == 1 )) {	/* ��޲����ѵ��ް			*/
			KSG_RauMdmConnect();										/* νĐڑ��v��				*/
			KSG_dials.BIT.ing = 1;										/* �޲�ْ�					*/
			KSG_f_host_conn_req = 0;									/*							*/
		}																/*							*/
		if( ( KSG_line_stat_h & KSG_DR_SIG ) == 0 ) {					/* DSR OFF?					*/
			KSG_mdm_status = 1;											/* ��DTE Ready				*/
			SCI7_RTS = 1; 												/* RTS OFF					*/
			KSG_dials.BIT.ing = 0;										/* �޲�ق��ĂȂ�			*/
			KSG_f_host_conn_req = 0;									/* νĐڑ��v���׸�			*/
			KSG_f_ReDial_rq = KSG_CLR;		KSG_f_ReDial_ov = KSG_SET;	/* ��޲����ϒ�~			*/
			KSG_f_DialConn_rq = KSG_CLR;	KSG_f_DialConn_ov = KSG_CLR;/* ����ڑ��҂���ϒ�~		*/
		}																/*							*/
		if( KSG_f_DialConn_ov == 1 ) {									/* ����ڑ��҂���ϵ��ް		*/
			KSG_NoReciveCnt++;											/* ���������ı���			*/
			if( KSG_NoReciveCnt > KSG_RauConf.Dpa_cnct_rty_cn ){		/* ���������ĵ��ް			*/
				KSG_f_mdm_conn_err = 1;									/* νĐڑ��װ				*/
				KSG_mdm_init_sts = 0;									/* ���я����������~		*/
				KSG_RauModem_OFF();										/*							*/
			}															/*							*/
			else {														/*							*/
				KSG_f_mdm_conn_err = 1;									/* ��νĐڑ��װ				*/
				KSG_mdm_status = 2;										/* DTE-DCE�m��				*/
				KSG_dials.BIT.ing = 0;									/* �޲�ق��ĂȂ�			*/
				KSG_f_host_conn_req = 0;								/* νĐڑ��v���׸�			*/
			}															/*							*/
			KSG_f_ReDial_rq = KSG_CLR;		KSG_f_ReDial_ov = KSG_SET;	/* ��޲����ϒ�~			*/
		}																/*							*/
		if ( KSG_GetInterval( ConnTimeOutTimer ) > (70L * 1000L) ){
			// PPP�ڑ��s�ǔ���
			RauSysErrSet( RAU_ERR_FMA_PPP_TIMEOUT, 1 );
		}
		if ( KSG_f_ReDialTime_ov == 1 ){
			KSG_Tm_ReDialTime.tm = 0;
		}
		break;															/*							*/

	/* --- �ʐM�� ------------------------------------------------------------------------------*/
	case 4:																/*							*/
		KSG_Tm_ReDialTime.tm = 0;
		KSG_Cnt_Retry =0;												/* �ڑ���ײ���� ؾ��		*/
		KSG_NoReciveCnt = 0;											/* �����������ر			*/
		KSG_f_ReDial_rq = KSG_CLR;	KSG_f_ReDial_ov = KSG_SET;			/* ��޲����ϒ�~			*/
		KSG_f_mdm_conn_err = 0;											/* νĐڑ��װ����			*/
		KSG_RauModemOnline = 1;
		if( ( KSG_line_stat_h & KSG_CD_SIG ) == 0 ) {					/* ����ؒf					*/
			KSG_mdm_status = 2;											/* DTE-DCE�m��				*/
			KSG_RauModemOnline = 0;										/* νĉ������׸� �ؒf		*/
			KSG_dials.BIT.out = KSG_dials.BIT.in = 0;					/* �޲�ٽð�� �ر			*/
			KSG_f_init_repeat = 0;										/* �������� ؾ��			*/
			f_OK = 1;													/*							*/
			KSG_Tm_AtCom.BIT.f_ov = KSG_SET;							/*							*/
			KSG_mdm_mode = 0;											/* �����Ӱ��				*/
			KSG_Tm_Gurd.BIT.bit0 = (ushort)KSG_Tm_Gurd_COUNT;			/* �޲�ٗ}����ϒl���		*/
			KSG_f_Gurd_rq = KSG_SET;	KSG_f_Gurd_ov = KSG_CLR;		/* �޲�ٗ}����ϊJ�n			*/
			KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
		}																/*							*/
		if( ( KSG_line_stat_h & KSG_DR_SIG ) == 0 ) {					/* DSR OFF					*/
			KSG_RauModemOnline = 0;										/* νĉ������׸� �ؒf		*/
			KSG_dials.BIT.out = KSG_dials.BIT.in = 0;					/* �޲�ٽð�� �ر			*/
			KSG_mdm_status = 1;											/* DTE Ready				*/
			SCI7_RTS = 1; 												/* RTS OFF					*/
			KSG_Tm_Gurd.BIT.bit0 = (ushort)KSG_Tm_Gurd_COUNT;			/* �޲�ٗ}����ϒl���		*/
			KSG_f_Gurd_rq = KSG_SET;	KSG_f_Gurd_ov = KSG_CLR;		/* �޲�ٗ}����ϊJ�n			*/
			KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
		}																/*							*/
		break;															/*							*/

	case 10:															/* �������فA��M�d�͎w�W�擾 */
		KSG_RauMdmAntLevMode();
		break;
	case 11:
		break;
	/* --- ���̑� ------------------------------------------------------------------------------*/
	default:															/*							*/
		if( KSG_f_Gurd_ov == 1 ) {										/*							*/
			if( KSG_mdm_init_sts > 0 ) KSG_RauModem_ON();				/* ���я��������쒆			*/
		}																/*							*/
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
		break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
	}																	/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	�A���e�i���x���A��M�d�͎w���擾														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmAntLevMode();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauMdmAntLevMode( void )
{
	// 
	switch( KSG_mdm_ant_sts ){
	case 1:	//+++
	case 2: //ATH
	case 3: //AT*DRPW
	case 4: //AT*DANTE
		if( KSG_uc_AtResultWait == 0 && KSG_Tm_AtCom.BIT.f_ov == 1 ) {
			KSG_uc_AtResultWait = 1;
			//	�����������
			mdm_result_f.BYTE = 0;
			KSG_RauMdmAntLevelCmdSetFOMA( KSG_mdm_ant_sts );
			//	��ϒl�ݒ�
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
			if(KSG_mdm_ant_sts == 2) {
				KSG_Tm_AtCom.BIT.bit0 = (ushort)-400;		// 40s timer set.For Result Wait.
			}
			else {
				KSG_Tm_AtCom.BIT.bit0 = (ushort)-30;		// 3s timer set.For Result Wait.
			}
			KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
			KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
		}
		if ( KSG_uc_AtResultWait == 1 && (f_OK == 1 || f_NOCON == 1)){
			// �����҂��� OK ��M
			KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
			KSG_Tm_AtCom.BIT.bit0 = (ushort)-1;
			KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
			KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
			KSG_uc_AtResultWait = 0;
			KSG_uc_AtRetryCnt = 0;
			KSG_mdm_ant_sts ++;
		}
		if ( KSG_uc_AtResultWait == 1 && KSG_Tm_AtCom.BIT.f_ov == KSG_SET ){ // �����҂� Time Out //
			KSG_uc_AtResultWait = 0;
			if(KSG_mdm_ant_sts != 1){
				KSG_mdm_ant_sts = 1;
			}
			else{
				KSG_mdm_ant_sts = 3;
			}
			KSG_Tm_AtCom.BIT.f_ov = KSG_SET;		// Retry from 1 //
		}
		break;
	case 5:
		KSG_gAntLevel = antena_level;
		KSG_gReceptionLevel = reception_level;
		KSG_mdm_ant_sts = 3;
		queset( OPETCBNO, KSG_ANT_EVT, 0, NULL );
		KSG_Tm_AtCom.BIT.f_rq = KSG_CLR;
		KSG_Tm_AtCom.BIT.bit0 = (ushort)antena_req_interval;
		KSG_Tm_AtCom.BIT.f_ov = KSG_CLR;
		KSG_Tm_AtCom.BIT.f_rq = KSG_SET;
		break;
	default:
		break;
	}
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ػ��ĺ�������																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmResultCheck();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmResultCheck(void)
{																		/*							*/

if (KSG_rx_save_len > 1) {												/* ��M�ް��L��				*/
		KSG_mdm_r_buf[ KSG_rx_save_len ] = 0;							/*							*/
		if		(	( KSG_mdm_r_buf[0] == 'O' ) &&						/* "OK"?					*/
					( KSG_mdm_r_buf[1] == 'K' ) ){						/*							*/
			f_OK = 1;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'C' ) &&						/* "CONN"?					*/
					( KSG_mdm_r_buf[1] == 'O' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'N' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'N' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_CONN = 1;													/*							*/
			KSG_mdm_mode = 1;											/* �ڑ�����Ӱ��				*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'B' ) &&						/* "BUSY"?					*/
					( KSG_mdm_r_buf[1] == 'U' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'S' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'Y' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_BUSY = 1;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'D' ) &&						/* "DELA"?					*/
					( KSG_mdm_r_buf[1] == 'E' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'L' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'A' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_DELAYED = 1;												/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'N' ) &&						/* "NO"?					*/
					( KSG_mdm_r_buf[1] == 'O' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_NOCON = 1;												/*							*/
			KSG_mdm_mode = 0;											/* �����Ӱ��				*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'E' ) &&						/* "ERR"?					*/
					( KSG_mdm_r_buf[1] == 'R' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'R' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_ERR = 1;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == '+' ) &&						/* "AT+CNUM"?				*/
					( KSG_mdm_r_buf[1] == 'C' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'N' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'U' ) &&						/*							*/
					( KSG_mdm_r_buf[4] == 'M' ) &&						/*							*/
					( KSG_mdm_r_buf[5] == ':' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_ERR = 0;													/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'A' ) &&						/* "ATE0"?					*/
					( KSG_mdm_r_buf[1] == 'T' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'E' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == '0' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_ECHOBACK = 1;												/*							*/
		}																/*							*/
		else if	(	( KSG_mdm_r_buf[0] == 'R' ) &&						/* "RING"?					*/
					( KSG_mdm_r_buf[1] == 'I' ) &&						/*							*/
					( KSG_mdm_r_buf[2] == 'N' ) &&						/*							*/
					( KSG_mdm_r_buf[3] == 'G' ) ){						/*							*/
			mdm_result_f.BYTE = 0x00;									/*							*/
			f_RING  = 1;												/*							*/
		}																/*							*/
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_mdm_r_buf[0] == 'R' ) &&		// RESTRICTION //
				 ( KSG_mdm_r_buf[1] == 'E' ) &&
				 ( KSG_mdm_r_buf[2] == 'S' ) &&
				 ( KSG_mdm_r_buf[3] == 'T' ) &&
				 ( KSG_mdm_r_buf[4] == 'R' ) &&
				 ( KSG_mdm_r_buf[5] == 'I' ) &&
				 ( KSG_mdm_r_buf[6] == 'C' ) &&
				 ( KSG_mdm_r_buf[7] == 'T' ) &&
				 ( KSG_mdm_r_buf[8] == 'I' ) &&
				 ( KSG_mdm_r_buf[9] == 'O' ) &&
				 ( KSG_mdm_r_buf[10]== 'N' ) ){

			mdm_result_f.BYTE = 0x00;
			f_NOCON = 1;
			f_resFomaRestriction = 1;
			KSG_mdm_mode = 0;							  // �R�}���h���[�h
		}
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_rx_save_len >= (4+2) ) &&
				 ( KSG_mdm_r_buf[0] == 'F' ) &&
				 ( KSG_mdm_r_buf[1] == 'O' ) &&
				 ( KSG_mdm_r_buf[2] == 'M' ) &&
				 ( KSG_mdm_r_buf[3] == 'A' )){
			if (KSG_rx_save_len >= (4+5+2) &&
				 ( KSG_mdm_r_buf[4] == ' ' ) &&
				 ( KSG_mdm_r_buf[5] == 'U' ) &&
				 ( KSG_mdm_r_buf[6] == 'M' ) &&
				 ( KSG_mdm_r_buf[7] == '0' )){
				if (KSG_uc_AdapterType == 0 &&
					KSG_mdm_r_buf[8] == '2') {
					f_TYPE_OK = 1;
				}
				else if (KSG_uc_AdapterType == 1 &&
					KSG_mdm_r_buf[8] == '3') {
					f_TYPE_OK = 1;
				}
			}
		}
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_uc_AdapterType == 2 ) && // UM04-KO
				 ( KSG_mdm_r_buf[0] == 'U' ) &&
				 ( KSG_mdm_r_buf[1] == 'M' ) &&
				 ( KSG_mdm_r_buf[2] == '0' ) &&
				 ( KSG_mdm_r_buf[3] == '4' ) &&
				 ( KSG_mdm_r_buf[4] == '-' ) &&
				 ( KSG_mdm_r_buf[5] == 'K' ) &&
				 ( KSG_mdm_r_buf[6] == 'O' ) ){
					f_TYPE_OK = 1;
		}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_rx_save_len >= (7+2) ) &&
				 ( KSG_mdm_r_buf[0] == '*' ) &&
				 ( KSG_mdm_r_buf[1] == 'D' ) &&
				 ( KSG_mdm_r_buf[2] == 'A' ) &&
				 ( KSG_mdm_r_buf[3] == 'N' ) &&
				 ( KSG_mdm_r_buf[4] == 'T' ) &&
				 ( KSG_mdm_r_buf[5] == 'E' ) &&
				 ( KSG_mdm_r_buf[6] == ':' )){

			// UM03-KO�̏ꍇ�A�R�����̌�ɃX�y�[�X������̂œǂݎ̂Ă�
		 	if (KSG_mdm_r_buf[7] == ' ') {
		 		KSG_mdm_r_buf[7] = KSG_mdm_r_buf[8];
		 		KSG_rx_save_len--;					// �����𒲐�����
		 	}
			if ( KSG_rx_save_len == (7+2) ){		// "*DANTE:CrLf" = 0 �����Ƃ��� //
				KSG_mdm_r_buf[7] = '0';
			}
			KSG_RauAntenaLevelSet();
		}
		else if (( KSG_uc_FomaFlag == 1 ) &&
				 ( KSG_rx_save_len >= (6+2) ) &&
				 ( KSG_mdm_r_buf[0] == '*' ) &&
				 ( KSG_mdm_r_buf[1] == 'D' ) &&
				 ( KSG_mdm_r_buf[2] == 'R' ) &&
				 ( KSG_mdm_r_buf[3] == 'P' ) &&
				 ( KSG_mdm_r_buf[4] == 'W' ) &&
				 ( KSG_mdm_r_buf[5] == ':' )){

			// UM03-KO�̏ꍇ�A�R�����̌�ɃX�y�[�X������̂œǂݎ̂Ă�
		 	if ( KSG_mdm_r_buf[6] == ' ' ) {
	 			KSG_mdm_r_buf[6] = KSG_mdm_r_buf[7];
	 			KSG_mdm_r_buf[7] = KSG_mdm_r_buf[8];
		 		KSG_rx_save_len--;					// �����𒲐�����
		 	}
			if ( KSG_rx_save_len == (6+2) ){		// "*DRPW:CrLf" = 0 �����Ƃ��� //
				KSG_mdm_r_buf[6] = '0';
				KSG_mdm_r_buf[7] = '0';
				// �A���e�i���x���G���[(E7735,E7736,E7779)�ɕt������d�͎w���l���擾�ł��Ȃ��ꍇ��FF���Z�b�g
				if( KSG_mdm_status != 10 ) {
					// �����e�i���X����̃A���e�i���x���擾���łȂ����FF���Z�b�g
					reception_level = 0xFF;
					return;
				}
			}
			if ( KSG_rx_save_len == (7+2) ){
				KSG_mdm_r_buf[7] = KSG_mdm_r_buf[6];
				KSG_mdm_r_buf[6] = '0';
			}
			KSG_RauReceptionLevelSet();
		}
	}																	/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	FOMA�A���e�i���x���擾																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauAntenaLevelSet();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauAntenaLevelSet(void)
{

	f_resFomaAntenaLow = 1;

	if ( KSG_mdm_r_buf[7] == '0' ){	// �A���e�i���x���O
		antena_level = LEVEL_0;
		antena_req_cnt = 0;
	}
	else if ( KSG_mdm_r_buf[7] == '1' ){	// �A���e�i���x���P
		antena_level = LEVEL_1;
		antena_req_cnt = 0;
	}
	else if ( KSG_mdm_r_buf[7] == '2' ){	// �A���e�i���x���Q
		antena_level = LEVEL_2;
		antena_req_cnt = 0;
	}
	else if ( KSG_mdm_r_buf[7] == '3' ){	// �A���e�i���x���R
		antena_level = LEVEL_MAX;
		f_resFomaAntenaLow = 0;
		antena_req_cnt = 0;
	}
	else{
		// �d�����������DANTE�R�}���h�����s����ƃ��U���g�̐���(0�`3)���Z�b�g����Ȃ����Ƃ�����
		/* 	UM02-F���f���́AFOMA�����ɓ����Ă���20�`30�b���x�o�߂��Ȃ���DANTE�R�}���h�ɑ΂���A���e�i���x���l���Z�b�g���Ȃ������B
			�����ŁA�]�T�����Ĕ{�̖�60�b�ԁA�A���e�i���x�����Z�b�g�����̂�҂悤�ɂ����B*/
		if(++antena_req_cnt >= ANTENA_REQCNT_MAX ){
			antena_level = LEVEL_0;		// 60��DANTE�R�}���h�����s���Ă��A���e�i���x�����擾�ł��Ȃ��ꍇ�͌��O�ƌ��Ȃ�
			antena_req_cnt = 0;
		}else{
			f_resFomaAntenaLow = 0;
		}
	}
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	FOMA��M�d�͎w�W�擾																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauReceptionLevelSet();												   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void KSG_RauReceptionLevelSet(void)
{
	unsigned short reception1;
	unsigned short reception2;

	if( (KSG_mdm_r_buf[6] >= '0') &&
		(KSG_mdm_r_buf[6] <= '9') &&
		(KSG_mdm_r_buf[7] >= '0') &&
		(KSG_mdm_r_buf[7] <= '9') ){
		reception1 = KSG_mdm_r_buf[6] - 0x30;
		reception2 = KSG_mdm_r_buf[7] - 0x30;

		reception_level = (reception1*10) + reception2;
		antena_req_cnt = 0;
	}
	else{
		// �d�����������DRPW�R�}���h�����s����ƃ��U���g�̐������Z�b�g����Ȃ��΍�
		if(++antena_req_cnt >= ANTENA_REQCNT_MAX ){
			reception_level = 0;		// 60��DRPW�R�}���h�����s���Ă���M�d�͎w�W���擾�ł��Ȃ��ꍇ�̓��x��0�ƌ��Ȃ�
			antena_req_cnt = 0;
		}
	}
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���ѐڑ��v������																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModem_ON();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModem_ON(void)
{																		/*							*/
	KSG_code_sts = 0x00;												/*							*/
	KSG_ptr_rx_save = &KSG_mdm_r_buf[0];								/* ��M�ް��i�[				*/
	KSG_rx_save_len = 0;												/* ��M�ް��i�[���ر		*/
	SCI7_DTR = 0;														/* ER(DTR) ON				*/
	KSG_mdm_status = 1;													/* DTE Ready				*/
//	if ( KSG_RauConf.ConnectFlag == 1 ) {
//		KSG_RauConf.ConnectFlag = 0;									// '�ڑ��Ȃ�' �� '����'�ɕς���
//		KSG_mdm_status = 5;												// Standby
//		KSG_DownLoadStandbyFlag = 5;									// ����޲
//		//����޲�� n ���p��������ʏ�ɖ߂�//
//		KSG_DownLoadSafeTime = (long)((long)KSG_RauConf.ExitStandbySafeTime * 10L * 60L);
//	}
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���ѐؒf�v������																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModem_OFF();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModem_OFF(void)
{																		/*							*/
	if ( KSG_uc_FomaFlag == 1 ){
		RAUhost_TcpConnReq_Clear ( );
		KSG_RauModem_OFF_2 ( );
		return;
	}
	SCI7_RTS = 1;														/* RS(RTS) OFF				*/
	SCI7_DTR = 1;														/* ER(DTR) OFF				*/
	KSG_mdm_init_sts = 0;												/* ���я����������~		*/
	KSG_modem_req = 0;													/* modem�ڑ�ظ����׸�		*/
	KSG_mdm_status = 0;													/* DTE not Ready			*/
	KSG_RauModemOnline = 0;												/* νĉ������׸� �ؒf		*/
	KSG_dials.BYTE = 0;													/* �޲�ٽð�� ؾ��			*/
	KSG_RauMdmSndQueInit();												/*							*/
	KSG_Tm_Gurd.BIT.bit0 = (ushort)KSG_Tm_Gurd_COUNT;					/* �޲�ٶް����ϐݒ�		*/
	KSG_f_Gurd_rq = KSG_SET;	KSG_f_Gurd_ov = KSG_CLR;				/* �޲�ٶް����ϊJ�n		*/
	KSG_Tm_Reset_t = 50;												/*							*/

}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���ѐؒf�v������-2																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauModem_OFF_2();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauModem_OFF_2(void)
{																		/*							*/
	SCI7_RTS = 1;														/* RS(RTS) OFF				*/
	SCI7_DTR = 1;														/* ER(DTR) OFF				*/
	KSG_mdm_init_sts = 0;												/* ���я����������~		*/
	KSG_modem_req = 0;													/* modem�ڑ�ظ����׸�(���g�p)*/
	KSG_mdm_status = 0;													/* DTE not Ready			*/
	KSG_uc_AtResultWait = 0;											/* AT Restult Wait Flag Clear */
	KSG_RauModemOnline = 0;												/* νĉ������׸� �ؒf		*/
	KSG_dials.BYTE = 0;													/* �޲�ٽð�� ؾ��			*/
	KSG_RauMdmSndQueInit();												/*							*/

	KSG_uc_FomaPWRphase = 1;											// (1)CD OFF �Ď��� //
	KSG_Tm_Reset_t = KSG_FomaModemPwrPhase1;							// 60�b��� CD OFF �m�F //
	f_ErrSndReqERRCD = 0;
	f_ErrSndReqMDMOFF = 0;
	KSG_Tm_Gurd.tm = 0;													// T_ResetOutFOMA() ����ϰ��� //
	KSG_uc_FomaWaitDRFlag = 0;		// DR�Ď����Ă��Ȃ�

	return;
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���я������v������																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmInitReq();															   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmInitReq(void)
{																		/*							*/
	KSG_mdm_init_sts = 1; 												/* ���я������v��			*/
	KSG_modem_req = 2;													/* modem�ڑ�ظ����׸�		*/
}																		/*							*/


//---------------------------------------------------------------------------------------------
// TelNum �� ���l�̂݃Z�b�g����.
//---------------------------------------------------------------------------------------------
short KSG_RauMdmInitCmdSetFOMA_SetTelNum ( unsigned char *setBuf, unsigned char *TelNum, int len )
{
	short i,s;
	for ( i=s=0; i < len; i++ ){
		if ( TelNum[i] >= '0' && TelNum[i] <= '9' ){
			setBuf[s++] = TelNum[i];
		}
	}
	return s;
}
//---------------------------------------------------------------------------------------------
// 8Bit x 4 �Ŋi�[����ꂢ�� IP Address�𕶎���ɂ��ā@. ��}�����Ċi�[����.
//---------------------------------------------------------------------------------------------
void KSG_RauMdmInitCmdSetFOMA_GetIpAdrStr ( unsigned char *setBuf, unsigned char *IPdat )
{
	//012.456.890.234
	intoas ( &setBuf[0], IPdat[0], 3 );
	setBuf[3] = '.';
	intoas ( &setBuf[4], IPdat[1], 3 );
	setBuf[7] = '.';
	intoas ( &setBuf[8], IPdat[2], 3 );
	setBuf[11] = '.';
	intoas ( &setBuf[12], IPdat[3], 3 );
}
//---------------------------------------------------------------------------------------------
// 12���� IP ������� . ��}�����ăZ�b�g����.
//---------------------------------------------------------------------------------------------
void KSG_RauMdmInitCmdSetFOMA_SetIpAdr ( unsigned char *setBuf, unsigned char *IPbuf )
{
	setBuf[0] = IPbuf[0];
	setBuf[1] = IPbuf[1];
	setBuf[2] = IPbuf[2];
	setBuf[3] = '.';
	setBuf[4] = IPbuf[3];
	setBuf[5] = IPbuf[4];
	setBuf[6] = IPbuf[5];
	setBuf[7] = '.';
	setBuf[8] = IPbuf[6];
	setBuf[9] = IPbuf[7];
	setBuf[10] = IPbuf[8];
	setBuf[11] = '.';
	setBuf[12] = IPbuf[9];
	setBuf[13] = IPbuf[10];
	setBuf[14] = IPbuf[11];
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���я���������ޑ��M����																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmInitCmdSetFOMA();												   |*/
/*| PARAMETER1  :   signed short InitPhase	:	������̪���									   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmInitCmdSetFOMA( signed short InitPhase )
{																		/*							*/

	unsigned char		of;

	switch( InitPhase ) {												/*							*/
		default:
// GG109800(S) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
//			KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
//			// memcpy�ł����̂��H
//			memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			if (prm_get(COM_PRM, S_CEN, 52, 1, 3) == 1) {// ���ݒ� 36-0052�C PPP�F��ID�p�X���[�h�L
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd2[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd2[InitPhase], KSG_mdm_cmd_len );
			} else {// �{������
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			}
// GG109800(E) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
			break;
		case FOMA_AT_PHASE_CNUM:		// AT+CNUM
			if ( KSG_RauConf.Dpa_IP_h.SEG[0] == 0 &&
				 KSG_RauConf.Dpa_IP_h.SEG[1] == 0 &&
				 KSG_RauConf.Dpa_IP_h.SEG[2] == 0 &&
				 KSG_RauConf.Dpa_IP_h.SEG[3] == 0 &&
				 KSG_RauConf.Dpa_port_h == 0 ){
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );	// AT&P
				memcpy( &KSG_mdm_cmd_buf[0], mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
				KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;			// �I��点�� //
				break;
			}
			KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );	// AT&P
			memcpy( &KSG_mdm_cmd_buf[0], mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			break;
		case FOMA_AT_PHASE_CGDCONT:
				//AT+CGDCONT=1,"PPP","//
				memcpy( &KSG_mdm_cmd_buf[0], KSG_RauMdmFmaATCGD_1, sizeof( KSG_RauMdmFmaATCGD_1 ) );
				of = KSG_RauConf.APNLen;
				memcpy ( &KSG_mdm_cmd_buf[20], &KSG_RauConf.APNName[0], of ); of += 20;
				memcpy ( &KSG_mdm_cmd_buf[of], "\"\r\n", 3 ); of += 3;
				KSG_mdm_cmd_len = of;
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
//				KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;			// �I��点�� //
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
			break;
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
		case FOMA_AT_PHASE_KALWAYSON:// UM04-KO�ŗL����
// GG109800(S) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
//			KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
//			memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			if (prm_get(COM_PRM, S_CEN, 52, 1, 3) == 1) {// ���ݒ� 36-0052�C PPP�F��ID�p�X���[�h�L
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd2[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd2[InitPhase], KSG_mdm_cmd_len );
			} else {// �{������
				KSG_mdm_cmd_len = strlen( mdmFoMa_cmd[InitPhase] );
				memcpy( &KSG_mdm_cmd_buf, mdmFoMa_cmd[InitPhase], KSG_mdm_cmd_len );
			}
// GG109800(E) A.Iiizumi 2020/01/20 LTE �A�}�m���iPPP�F��ID�p�X���[�h�L�j�Ή��i�ݒ�Ő؊����\�Ƃ���36-0052�C�j
			KSG_mdm_init_sts = FOMA_AT_PHASE_MAX - 1;//(FOMA_AT_PHASE_KALWAYSON���Ō�̃R�}���h)
			break;
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
	}
	KSG_f_mdm_kind_err = 0;												/*							*/
	KSG_RauMdmTxStart();												/*							*/
}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	�������فA��M�d�͎w���擾����ޑ��M����													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmAntLevelCmdSetFOMA();											   |*/
/*| PARAMETER1  :   signed short phase	:	̪���											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
const char *mdmFoMa_AntLevCmd[5] = {
	"",
	"+++",
	"ATH0\r\n",		// ����ؒf
	"AT*DRPW\r\n",	// ��M�d�͎w���擾
	"AT*DANTE\r\n"	// �A���e�i���x���擾
};

void	KSG_RauMdmAntLevelCmdSetFOMA( signed short phase )
{
	KSG_mdm_cmd_len = strlen( mdmFoMa_AntLevCmd[phase] );
	memcpy( &KSG_mdm_cmd_buf, mdmFoMa_AntLevCmd[phase], KSG_mdm_cmd_len );
	KSG_f_mdm_kind_err = 0;
	KSG_RauMdmTxStart();

}

/*[]------------------------------------------------------------------------------------------[]*/
/*|	���Ѻ���ޑ��M�J�n����																	   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmTxStart();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmTxStart( void )
{																		/*							*/
	KSG_mdm_cmd_buf[KSG_mdm_cmd_len] = 0;								/*							*/
	KSG_mdm_cmd_ptr = 0;												/* ���Ѻ���ޑ��M�ް� �擪	*/
	SCI7.TDR = KSG_mdm_cmd_buf[KSG_mdm_cmd_ptr];						/* 1 byte���M				*/
	KSG_mdm_cmd_ptr++;													/* ���Ѻ���ޑ��M�ް� ���Z	*/
	SCI7.SCR.BIT.TIE = 1;												/* ���M�ް��󂫊����݋���	*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	νĐڑ��v������																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmConnect();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmConnect( void )
{
	if ( KSG_uc_FomaFlag == 1 ){
		strcpy((char *)&KSG_mdm_cmd_buf[0], "ATD*99***1#\r\n");
		KSG_mdm_cmd_len = strlen( (char*)KSG_mdm_cmd_buf );
		KSG_mdm_cmd_ptr = 0x00;
		f_resFomaRestriction = 0;

		KSG_RauMdmTxStart();			// ���f���R�}���h���M�J�n

		// ����ڑ��҂��^�C�}�ݒ�
		if ( KSG_RauConf.Dpa_dial_wait_tm ){
			KSG_Tm_DialConn.BIT.bit0 = -( KSG_RauConf.Dpa_dial_wait_tm * 10 );/* �_�C�A�������҂���ϐݒ�		*/
		}else{
			KSG_Tm_DialConn.BIT.bit0 = (ushort)-1250;					// 125 S // 2008/11/19.
		}
		KSG_f_DialConn_rq = KSG_SET;
		KSG_f_DialConn_ov = KSG_CLR;
		return;
	}
//	strcpy((char *)&KSG_mdm_cmd_buf[0], "ATD");							/* ���Ѻ���ޓ]��			*/
//	memcpy( &KSG_mdm_cmd_buf[3], &KSG_RauActTBL.TelNumber[0], 12 );		/* HOST�d�b�ԍ�				*/
//	KSG_mdm_cmd_buf[15] = KSG_CR;	KSG_mdm_cmd_buf[16] = KSG_LF;		/* CR/LF�̕t��				*/
//	KSG_mdm_cmd_len = 17;												/* ���Ѻ���ޑ��M�ް������	*/
//	KSG_mdm_cmd_ptr = 0;												/* ���Ѻ���ޑ��M�ް��擪	*/
//	KSG_RauMdmTxStart();												/* ���Ѻ���ޑ��M�J�n		*/
//	if( KSG_RauActTBL.DialWait > 0 ) {									/*							*/
//		KSG_Tm_DialConn.BIT.bit0 = -( KSG_RauActTBL.DialWait * 10 );	/* ����ڑ��҂���ϐݒ�		*/
//		KSG_f_DialConn_rq = KSG_SET;	KSG_f_DialConn_ov = KSG_CLR;	/*							*/
//	}																	/*							*/
//	else {																/*							*/
//		KSG_Tm_DialConn.tm = 0;											/*							*/
//	}																	/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	RS232C����M�� ������ر����������														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmSigInfoInit();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmSigInfoInit( void )
{																		/*							*/
	KSG_DSR = 1;														/* DSR off					*/
	KSG_CD  = 1;														/* CD  off					*/
	KSG_CTS = 1;
	KSG_CI = 1;
	KSG_Chatt_DSR = 0xFF;
	KSG_Chatt_CTS = 0xFF;
	KSG_Chatt_CD = 0xFF;
	KSG_Chatt_CI = 0xFF;
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	�@��ؾ�ďo�͏���																		   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauTResetOut();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauTResetOut( void )
{																		/*							*/
	if ( KSG_uc_FomaFlag == 1 ){
		KSG_RauTResetOutFOMA();
		return;
	}
	if( KSG_Tm_Reset_t != 0x00 ) {										/*							*/
		KSG_Tm_Reset_t--;												/*							*/
		if( KSG_Tm_Reset_t == 0x00 ) {
			SCI7_RST = 0;												/* �@��ؾ�ďo��(OFF)		*/
		}
		else {
			SCI7_RST = 1;												/* �@��ؾ�ďo��(ON)			*/
		}
	}																	/*							*/
}																		/*							*/
																		/*							*/
/*[]----------------------------------------------------------------------[]*/
/*|             KSG_RauTResetOutFOMA()                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*|         �@�탊�Z�b�g�o�͏��� & DR�Ď�����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-09-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	KSG_RauTResetOutFOMA( void )
{
	switch(KSG_uc_AdapterType) {
	default:
		KSG_RauTResetOutFOMA_NTNET();
		break;
	case	1:
		KSG_RauTResetOut_UM03();
		break;
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
	case	2:
		KSG_RauTResetOut_UM04();
		break;
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*|             KSG_RauTResetOutFOMA_NTNET()                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|         �@�탊�Z�b�g�o�͏��� & DR�Ď�����                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-09-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
//�� ���L�֐��̎��ԃp�����[�^�́AUM02-KO�̓d��ON/OFF�V�[�P���X�ƍ���Ȃ������̂܂܂Ƃ���
void KSG_RauTResetOutFOMA_NTNET( void )							/* �@��ؾ�ďo�͏���			*/
{
	if (KSG_Tm_Reset_t != 0) {
		KSG_Tm_Reset_t--;

		// LAN / �ر� �̒ʐM�ؑւɔ���.OFF --> ON �̊Ԋu�𑬂�����.�K���҂��Ȃ��ŐM��������,���ɐi�߂�ꍇ�͐i��.
		switch(KSG_uc_FomaPWRphase){
		case	1:						// CD �Ď� //
			if( KSG_Tm_Reset_t <= 0 || KSG_CD != 0 ){ //�^�C���A�E�g���ACD OFF �܂� //

										// MODEM�̓d��OFF�v������ //
				f_ErrSndReqMDMOFF = 1;	// 2008/09/09

				if ( KSG_CD == 0 ){			// CD LOW�ɂȂ�Ȃ� //
					f_ErrSndReqERRCD = 1;// 2008/09/09
				}
				KSG_uc_FomaPWRphase = 2;
				SCI7_RST = 1;			// �@�탊�Z�b�g�o��(ON) [MODEM���猩���Ӗ��� POWER OFF]
				KSG_Tm_Reset_t = KSG_FomaModemPwrPhase2; //(20S)
			}
			break;
		// ̪��ނ�1�ǉ�. 1=CD Off�҂� 2=DSR Off�҂� 3=ON�܂ł̲������ 3S
		case	2:
			if( KSG_Tm_Reset_t <= 0 || KSG_DSR != 0 ){	// �^�C���A�E�g�� DSR OFF�܂�.
				KSG_uc_FomaPWRphase = 3;
				KSG_Tm_Reset_t = KSG_FomaModemPwrPhase3; //(5S) ON�܂ł̲������ .
			}
			break;
		case	3:
			if( KSG_Tm_Reset_t <= 0 ){
				// Time UP //
				SCI7_RST = 0;			// �@�탊�Z�b�g�o�͒�~(OFF) [MODEM���猩���Ӗ��� POWER ON]
					// �_�C�����K�[�h�^�C�}�ݒ� //
					KSG_Tm_Gurd.BIT.bit0 = (unsigned short)-20; //(2S)[2 + DSR_ON����AT�܂ł̊Ԋu 3 = 5S]
					KSG_f_Gurd_rq = KSG_SET;
					KSG_f_Gurd_ov = KSG_CLR;
					KSG_uc_FomaPWRphase = 0;
			}
			break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|				KSG_RauTResetOut_UM03()										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|			�@�탊�Z�b�g�o�͏��� & DR�Ď�����								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author		:	S.Fujii													|*/
/*|	Date		:	2015-05-22												|*/
/*|	Update		:															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void KSG_RauTResetOut_UM03( void )
{
// PWR�p���X����
	if (KSG_Tm_ResetPulse != 0) {
		if (--KSG_Tm_ResetPulse == 0) {
			SCI7_RST = 0;		// PWR=OPEN
		}
	}

// ���Z�b�g�V�[�P���X���{
	if (KSG_Tm_Reset_t != 0) {
		KSG_Tm_Reset_t--;

		switch(KSG_uc_FomaPWRphase){
		default:
			break;
		case	1:
		// CD OFF�Ď��@�FUM03-KO�d�l�ɂ͖������A�]���̘_���𓥏P���Ă���
			if (KSG_Tm_Reset_t <= 0 || KSG_CD != 0) {			// �^�C���A�E�g���ACD OFF�܂�

				f_ErrSndReqMDMOFF = 1;			// MODEM�̓d��OFF�v������ //
				if (KSG_CD == 0){
					f_ErrSndReqERRCD = 1;		// CD LOW�ɂȂ�Ȃ�
				}

				KSG_uc_FomaPWRphase = 2;
				KSG_Tm_Reset_t = UM03KO_PowerOffTime;			// (2S)
				SCI7_RST = 1;									// PWR=SG
				KSG_Tm_ResetPulse = UM03KO_PWR_PulseTime;		// (1S)
			}
			break;
		case	2:
		// CS & DR OFF�Ď�
			if (KSG_Tm_Reset_t <= 0 || KSG_DSR != 0) {			// �^�C���A�E�g�� DSR OFF�܂�
				KSG_uc_FomaPWRphase = 3;
				KSG_Tm_Reset_t = UM03KO_PowerOnDelay;			// (63S) ON�܂ł̲������ .
			}
			break;
		case	3:
		// PWR ON�҂�
		// UM03-KO�̓d�������PWR�p���X���͖��Ƀg�O���ω����邽�߁A�ʐM�P�[�u����d���P�[�u����
		// �����������邱�ƂŁA�\�t�g�̈Ӑ}���Ă���d����ԂƁA���ۂ̋@��d����Ԃɍ����o��ƁA
		// �񕜂ł��Ȃ��B���̂��߁A�d��ON�p�p���X�o�͑O��DR������ON�Ȃ�Ώ������X�L�b�v����B
		// �i���̏ꍇ���Z�b�g�ł��Ă��Ȃ��̂ŁA���ɖ����������Ƃ��ɍēx���Z�b�g����B�j
		// DR��ER��ON���Ă��Ȃ���ON�ɂȂ�Ȃ��̂Ń`�F�b�N�O�Ɉ�xER��ON����B
		// �iCD�ADR�Ȃǂ̐���M���T���v�����O�̂��߂Q�b�قǎ��Ԃ��󂯂�B�j
			if (KSG_Tm_Reset_t == 20) {							// �^�C���A�E�g�Q�b�O
				SCI7_DTR = 0;									// ER(DTR) ON�i�`�F�b�N�p�j
			}
			else if (KSG_Tm_Reset_t <= 0) {						// �^�C���A�E�g�܂�
				if (KSG_DSR != 0) {
				// DSR OFF�Ȃ�i����j�d��ON�p���X�o��
					SCI7_RST = 1;								// PWR=SG
					KSG_Tm_ResetPulse = UM03KO_PWR_PulseTime;	// (1S)
				}
				SCI7_DTR = 1;									// ER(DTR) OFF

				// �_�C�����K�[�h�^�C�}�ݒ�:UM03-KO��DR ON�Ȃ�΃R�}���h��t�Ȃ̂ōŏ��l�ŗǂ�
				KSG_Tm_Gurd.BIT.bit0 = (unsigned short)-UM03KO_PWR_PulseTime; // (1S)
				KSG_f_Gurd_rq = KSG_SET;
				KSG_f_Gurd_ov = KSG_CLR;
				KSG_uc_FomaPWRphase = 0;
			}
			break;
		}
	}
}
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
/*[]-----------------------------------------------------------------------[]*/
/*|				KSG_RauTResetOut_UM04()										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|			�@�탊�Z�b�g�o�͏��� & DR�Ď����� AD-04S(UM04-KO)�p				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author		:	A.Iiizumi												|*/
/*|	Date		:	2019-01-23												|*/
/*|	Update		:															|*/
/*[]-------------------------------------- Copyright(C) 2015 AMANO Corp.---[]*/
void KSG_RauTResetOut_UM04( void )
{
	if (KSG_Tm_Reset_t != 0) {
		KSG_Tm_Reset_t--;

		// LAN / �ر� �̒ʐM�ؑւɔ���.OFF --> ON �̊Ԋu�𑬂�����.�K���҂��Ȃ��ŐM��������,���ɐi�߂�ꍇ�͐i��.
		switch(KSG_uc_FomaPWRphase){
		case	1:						// CD �Ď� //
			if( KSG_Tm_Reset_t <= 0 || KSG_CD != 0 ){ //�^�C���A�E�g���ACD OFF �܂� //

										
				f_ErrSndReqMDMOFF = 1;	// MODEM�̓d��OFF�v������ //

				if ( KSG_CD == 0 ){			// CD LOW�ɂȂ�Ȃ� //
					f_ErrSndReqERRCD = 1;
				}
				KSG_uc_FomaPWRphase = 2;
				SCI7_RST = 1;			// �@�탊�Z�b�g�o��(ON) [MODEM���猩���Ӗ��� POWER OFF]
				KSG_Tm_Reset_t = UM04KO_PowerOffTime; // UM04-KO CS&DR OFF�҂����ԁF10�b
			}
			break;
		case	2:
			if( KSG_Tm_Reset_t <= 0 || KSG_DSR != 0 ){	// �^�C���A�E�g�� DSR OFF�܂�.
				KSG_uc_FomaPWRphase = 3;
				KSG_Tm_Reset_t = UM04KO_PowerOnDelay; // UM04-KO CS OFF��PWR ON�҂����ԁF60�b
			}
			break;
		case	3:
			if( KSG_Tm_Reset_t <= 0 ){
				// Time UP //
				SCI7_RST = 0;			// �@�탊�Z�b�g�o�͒�~(OFF) [MODEM���猩���Ӗ��� POWER ON]
				// �_�C�����K�[�h�^�C�}�ݒ� //
				// PWR ON ����DTR ON������܂ł�wait���ԁF�K��͂Ȃ�����2S�Ƃ��Ă���(KSG_RauModem_ON()�N���҂�)
				KSG_Tm_Gurd.BIT.bit0 =  (unsigned short)-UM04KO_ER_ON;

				KSG_f_Gurd_rq = KSG_SET;
				KSG_f_Gurd_ov = KSG_CLR;
				KSG_uc_FomaPWRphase = 0;
			}
			break;
		}
	}
}
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
/*[]----------------------------------------------------------------------[]*/
/*|             KSG_RauGetAntLevel()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|         �A���e�i���x���擾����                                         |*/
/*|           mode     : 0=�J�n                                            |*/
/*|                    : 1=�I��                                            |*/
/*|           interval : ���M�Ԋu                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2012-09-10                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
short  KSG_RauGetAntLevel( unsigned short mode, unsigned short interval )
{
	unsigned long	StartTime;

	KSG_gAntLevel = -1;
	KSG_gReceptionLevel = -1;

	if(KSG_mdm_status < 3){									// ���f���N����
		return -1;
	}

	if(mode == 0){											// ���M�Ԋu�ݒ�
		// HOST�Ƒ���M���Ȃ�A���e�i���x���擾�J�n�s��
		if( RAUhost_GetSndSeqFlag() != RAU_SND_SEQ_FLG_NORMAL ||
			Credit_GetSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ||
			RAUhost_GetRcvSeqFlag() != RAU_RCV_SEQ_FLG_NORMAL ){
			return -1;
		}		
		RAU_SetAntennaLevelState(1);
		// TCP���ؒf�����̂�҂�
		StartTime = LifeTimGet();
		do {
			taskchg(IDLETSKNO);
		} while(LifePastTimGet(StartTime) < RAU_TCP_CLOSE_TIME);	// 5�b
		antena_req_interval = interval * -1;
	}

	if(mode == 0){
		KSG_mdm_status = 10;								// �A���e�i���x���擾���[�h
		KSG_uc_AtResultWait = 0;
		KSG_mdm_ant_sts = 1;
		KSG_RauModemOnline = 0;
	}
	else{
		KSG_AntennaLevelCheck = 0;
		RAU_SetAntennaLevelState(0);
		KSG_AntAfterPppStarted = 1;							// �A���e�i���x���`�F�b�N�ɂ��E7778�}�~�p
		KSG_RauModem_OFF();
		KSG_ClosePPPSession(gInterfaceHandle_PPP_rau);
	}
	return 0;
}
