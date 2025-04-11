/*[]----------------------------------------------------------------------[]*/
/*| �֐��^�錾                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : Hara                                                     |*/
/*| Date        : 2005-02-01                                               |*/
/*| UpDate      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _RKN_FUN_H_
#define _RKN_FUN_H_

#include	"mem_def.h"
#include	"rkn_cal.h"

																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern void		mc10(void);												/*�@�ݒ荀�ڈꊇ�Q�Ɓ@�@�@�@�@�@�@�@*/
extern void		mc101(void);											/*�@�ݒ荀�ڈꊇ�Q�Ɓi����ѕ����j�@*/
extern void		mc102(void);											/*�@�ݒ荀�ڈꊇ�Q�Ɓi�����ѕ����j�@*/
																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern char		cm13(char,short *);										/*�@���������͈̓`�F�b�N�@�@�@�@�@�@*/
extern char		cm14(short);											/*�@�[�N�`�F�b�N�@�@�@�@�@�@�@�@�@�@*/
extern void		cm27(void);												/*�@�t���O�N���A�@�@�@�@�@�@�@�@�@�@*/
extern short	cm33(char);												/*�@���t���ϊ��@�@�@�@�@�@�@�@�@�@�@*/
extern short	cm41(short);											/*�@�ݒ荀�ڎQ�Ɓ@�@�@�@�@�@�@�@�@�@*/
																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern void		et02(void);												/*�@�����v�Z�����@�@�@�@�@�@�@�@�@�@*/
extern void		et20(short);											/*�@���ԗ����z�Z�o�@�@�@�@�@�@�@�@�@*/
extern void		et2100(short);											/*�@����ї����v�Z�@�@�@�@�@�@�@�@�@*/
extern void		et2200(short);											/*�@�����ї����v�Z�@�@�@�@�@�@�@�@�@*/
																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern void		et40(void);												/*�@���ԗ����v�Z�����@�@�@�@�@�@�@�@*/
extern void		et42(void);												/*�@���Ԋ��������@�@�@�@�@�@�@�@�@�@*/
extern long		et421(void);											/*�@�ʏ펞�Ԋ����z�Z�o�@�@�@�@�@�@�@*/
extern long		et422(void);											/*�@������Ԋ����z�Z�o�@�@�@�@�@�@�@*/
extern void		et43(void);												/*�@�������������@�@�@�@�@�@�@�@�@�@*/
extern void		et44(void);												/*�@�񐔌������@�@�@�@�@�@�@�@�@�@�@*/
extern void		et45(void);												/*�@�T�[�r�X�������@�@�@�@�@�@�@�@�@*/
extern void		et46(void);												/*�@�v���y�C�h�J�[�h�����@�@�@�@�@�@*/
extern void		et47(void);												/*�@����������@�@�@�@�@�@�@�@�@�@�@*/
extern void		et471(short,short);										/*�@���ԑђ���������@�@�@�@�@�@�@�@*/
extern char		et4711(short,struct CAR_TIM *);							/*�@����X�V���Ɏ����Z�o�@�@�@�@�@�@*/
extern void		et4712(short);											/*�@���ԗ����Z�o�i����сj�@�@�@�@�@*/
extern void		et472(void);											/*�@�������Ԓ���������@�@�@�@�@�@�@*/
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
extern void		et48(void);												/*�@�����������@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�
extern void		et51(void);												/*�@�X�ԍ��ʊ��������@�@�@�@�@�@�@�@*/
extern void		et57(void);												/*�@�Ԏ�؂�ւ������@�@�@�@�@�@�@�@*/
extern void		et59(void);												/*�@�������������@�@�@�@�@�@�@�@�@�@*/
extern void		et90(void);												/*  �N���W�b�g�J�[�h */
extern void		et94(void);												/*�@�d�q�}�l�[����		�@�@�@�@�@�@*/
extern void		et95(void);
extern void		et96(void);
extern void		et97(void);
																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern void		et40_remote(void);										/*�@�����[�g����		�@�@�@�@�@�@*/
																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern char		ec081( char *, char * );								/*�@�L���J�n�I���͈̓`�F�b�N�@�@�@�@*/
extern char		ec09(void);												/*�@�����v�Z�v���쐬�i�u�k�j�@�@�@�@*/
extern char		ec191(char,char,char,char);								/*�@�Ԏ�ؑ֗����v�Z�v���쐬�@�@�@�@*/
																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern char		ec61(struct CAR_TIM *);									/*�@�����̌n�Z�o�����@�@�@�@�@�@�@�@*/
extern void		ec62(struct CAR_TIM *);									/*�@���t�P���J�������@�@�@�@�@�@�@�@*/
extern void		ec63(struct CAR_TIM *);									/*�@���t�P���J�㏈���@�@�@�@�@�@�@�@*/
extern short	ec64(struct CAR_TIM *,struct CAR_TIM *);				/*�@�N����������r�����@�@�@�@�@�@�@*/
extern void		ec65(short,short);										/*�@�����ő嗿�������@�@�@�@�@�@�@�@*/
extern void		ec65_1(char);											/*�@�����ő嗿�������@�@�@�@�@�@�@�@*/
extern short	hour612chk(short);										/*									*/
extern void		ec66(struct CAR_TIM *,long);							// ���t�X�V�����i���Z�j �����t�g�偦
extern char		ec67(short,struct CAR_TIM *,struct CAR_TIM *,char);		// �̌n����юZ�o       �������ǉ���
extern unsigned long	ec71a(struct CAR_TIM *,struct CAR_TIM *);		// ���ԍ��Z�o����(ɰ�ײ��)
extern	short	ta_st_sel( char, char );								// ����j���؊�(0:00�Ή�)
																		// 
extern long		ec68(long,short);										/*�@����Ő؏グ�؎̂ď����@�@�@�@�@*/
extern char		ec69(struct CAR_TIM *,short);							/*�@������L�������`�F�b�N�@�@�@�@�@*/
extern void		ec70(struct CAR_TIM *,long);							/*�@���t�X�V�����i���Z�j�@�@�@�@�@�@*/
extern unsigned long	ec71(struct CAR_TIM *,struct CAR_TIM *);		/*�@���ԍ��Z�o�����@�@�@�@�@�@�@�@�@*/
extern short 	ec72(struct CAR_TIM *,struct CAR_TIM *,struct CAR_TIM *);	/*�@���ԑу`�G�b�N�@�@�@�@�@�@�@*/
extern char		ec82(short,struct CAR_TIM *,struct CAR_TIM *);			/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
																		/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern void		ryo_cal( char, unsigned short );						/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern void		sryo_cal( ushort );										/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern char		SvsTimChk( ushort );									/*�@�T�[�r�X�^�C�����`�F�b�N�@�@�@�@*/
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
extern int		vl_funchg( char f_Button, m_gtticstp *mag );
extern void		vl_tikchg( void );
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
extern void		vl_lcd_tikchg( void );
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))

extern void		vl_carchg( ushort, uchar );								/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern short	vl_paschg( void );									/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern short	vl_mifpaschg( void );									/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern short	vl_svschk( m_gtservic * );								/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//extern	short	vl_sousai( ushort );
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
extern short	vl_prechg( m_gtprepid * );								/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern	short	vl_kaschg( m_gtservic *mag );
extern void		vl_scarchg( ushort );									/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
extern void		vl_passet( ushort );									/*�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/

extern void vl_cyushiset( ulong );

extern	int	is_paid_remote(Receipt_data* dat);
// �d�l�ύX(S) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
extern	int	is_ParkingWebFurikae( Receipt_data* dat );
// �d�l�ύX(E) K.Onodera 2016/11/02 ���Z�f�[�^�t�H�[�}�b�g�Ή�
extern	short	is_paid_syusei(Receipt_data* dat);
extern	void	ryo_SyuseiRyobufSet( void );

extern	char	GET_RYO_SETMODE(uchar syubetu, uchar taikei);		/* �O���������菈�� */
extern	void	Change_Ryokin_Data( void );
extern	char	GET_PARSECT_NO(char no);
extern	char Get_Pram_Syubet( char no );
extern	long Carkind_Param( char no, char syu, char len, char pos);
// MH810100(S) Y.Watanabe 2020/02/26 �Ԕԃ`�P�b�g���X(���������Ή�)
extern	void ClearRyoCalDiscInfo( void );
// MH810100(E) Y.Watanabe 2020/02/26 �Ԕԃ`�P�b�g���X(���������Ή�)
// MH810100(S) 2020/06/10 #4216�y�A���]���w�E�����z�Ԏ�ݒ��ʎ��Ԋ����ݒ莞��QR����(���Ԋ���)���s���Ă��K�p����Ȃ�
extern	void IncCatintime( struct	CAR_TIM		*wok_tm );
// MH810100(E) 2020/06/10 #4216�y�A���]���w�E�����z�Ԏ�ݒ��ʎ��Ԋ����ݒ莞��QR����(���Ԋ���)���s���Ă��K�p����Ȃ�

#endif	// _RKN_FUN_H_
