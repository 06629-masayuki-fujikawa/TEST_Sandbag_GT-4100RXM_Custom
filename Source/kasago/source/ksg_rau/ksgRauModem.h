/************************************************************************************************/
/*																								*/
/*	Ӽޭ�ٖ���	:ksgRauModem.h		:---------------------------------------------------------: */
/*	Ӽޭ�ټ����	:					: Modem �֘A											  : */
/*																								*/
/************************************************************************************************/

/* ksgRauModemMain.c																					*/
extern	void			KSG_RauMdmRcvQueInit( void );						/* ��M����̏�����			*/
extern	void			KSG_RauMdmRcvQueSet( unsigned char *, unsigned int );
																			/* ��M����̏��o			*/
extern	void			KSG_RauMdmSndQueInit( void );						/* ���M����̏�����			*/
extern	void			KSG_RauMdmSndQueSet( unsigned char *, unsigned int, unsigned char );
																			/* ���M����̏��o			*/
//extern  void			KSG_RauMdmTimerInit( void );
extern	unsigned char	KSG_RauMdmSndQueRead( unsigned char *pData, unsigned int *len, unsigned char *ch, unsigned char kind );
extern	unsigned char	KSG_RauMdmRcvQueRead( unsigned char *pData, unsigned short *len );
																			/* ���M����̓Ǐo			*/
extern	void			KSG_RauMdm_SumCheck( unsigned char *, unsigned short , unsigned char * );
																			/* ������					*/
extern	void			KSG_RauMdmPostPppClose( void );
extern	void			KSG_RauClosePPPSession( void );
extern	unsigned char	KSG_RauGetMdmCutState( void );
extern	void			KSG_RauSetMdmCutState( unsigned char modemCutRequestState );

/* ksgRauModemControl.c																					*/
extern	void			KSG_RauMdmControl( void );							/* ���ѐ���					*/
extern	void			KSG_RauMdmAntLevMode( void );						/* �������فA��M�d�͎w���擾 */
extern	void			KSG_RauMdmResultCheck( void );						/* ػ��ĺ��މ��			*/
extern	void			KSG_RauAntenaLevelSet( void );						/* �������َ擾				*/
extern	void			KSG_RauReceptionLevelSet( void );					/* ��M�d�͎w�W�擾			*/
extern	void			KSG_RauModem_ON( void );							/* ���ѐڑ��v��				*/
extern	void			KSG_RauModem_OFF( void );							/* ���ѐؒf�v��				*/
extern	void			KSG_RauModem_OFF_2( void );							/* ���ѐؒf�v��-2 �ް��đ���ϰ�N���Ȃ� */
extern	void			KSG_RauMdmInitReq( void );							/* ���я������v��			*/
extern	void			KSG_RauMdmInitCmdSetFOMA( signed short );			/* ���я���������ޑ��M		*/
extern	void			KSG_RauMdmAntLevelCmdSetFOMA( signed short );		/* �������فA��M�d�͎w���擾����ޑ��M */
extern	void			KSG_RauMdmTxStart( void );							/* ���Ѻ���ޑ��M�J�n		*/
extern	void			KSG_RauMdmConnect( void );							/* νĐڑ��v��				*/
extern	void			KSG_RauMdmSigInfoInit( void );						/* RS232C ������ر������	*/
extern	void			KSG_RauTResetOut( void );							/* �@��ؾ�ďo�͏���			*/
extern	void			KSG_RauTResetOutFOMA( void );						/* �@��ؾ�ďo�͏���			*/
extern	void			KSG_RauTResetOutFOMA_NTNET( void );					/* �@��ؾ�ďo�͏���			*/
extern	void			KSG_RauTResetOut_UM03( void );						/* �@��ؾ�ďo�͏���(UM03-KO)*/
// MH322918(S) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������
extern	void			KSG_RauTResetOut_UM04( void );						/* �@��ؾ�ďo�͏���(UM04-KO)*/
// MH322918(E) A.Iiizumi 2019/03/11 LTE AD-04S(UM04-KO)�Ή� ���f������

/* ksgRauSci.c																							*/
extern	void			KSG_RauSciLineCheck( void );						/* ײݏ�Ԋm�F				*/
extern	unsigned short	KSG_CD_ON( void );									/* 							*/
extern	unsigned short 	KSG_CS_ON( void );									/*							*/

/* ksgRauModemDriver.c																					*/
extern	void			KSG_RauMdmTimerInit( void );						/* ��Ϗ���������			*/
extern	void			KSG_RauMdm10msInt( void );							/* 10ms��Ϗ���				*/
extern	void			KSG_RauMdm100msInt( void );							/* 100ms��Ϗ���				*/

/* ksgRauSysError.c																						*/
extern	void			KSG_RauSysErrSet( unsigned char n , unsigned char s );


#define RAU_ERR_FMA_ANTENALOW1		31										/* �ڑ����̱������ق� 1		*/
#define RAU_ERR_FMA_ANTENALOW2		32										/* �ڑ����̱������ق� 2		*/
#define	RAU_ERR_FMA_NOCARRIER		34										/* NO CARRIRE���o			*/
#define	RAU_ERR_FMA_SIMERROR		35										/* SIM�J�[�h�O�ꌟ�m		*/
#define	RAU_ERR_FMA_ADAPTER_NG		36										/* FOMA�A�_�v�^�ݒ�s��		*/
#define	RAU_ERR_FMA_MODEMERR_DR		74										/* DR�M���� 3�b�o�߂��Ă�ON�ɂȂ�Ȃ�(MODEM ON��) */
#define	RAU_ERR_FMA_MODEMERR_CD		75										/* CD�M����60�b�o�߂��Ă�OFF�ɂȂ�Ȃ�(MODEM OFF��) */
#define	RAU_ERR_FMA_PPP_TIMEOUT 	76										/* PPP�R�l�N�V�����^�C���A�E�g */
#define	RAU_ERR_FMA_RESTRICTION		77										/* RESTRICTION(�K����)����M */
#define RAU_ERR_FMA_MODEMPOWOFF		78										/* MODEM �d��OFF			*/
#define RAU_ERR_FMA_ANTENALOW		79										/* �ڑ����̱������ق� 0		*/
#define	RAU_ERR_FMA_REDIALOV		94										/* ���_�C�A���񐔃I�[�o�[	*/
