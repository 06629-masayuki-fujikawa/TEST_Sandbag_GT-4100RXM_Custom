/*************************************************************************/ /**
 *	\file	SodiacAPI.h
 *	\brief	Sodiac �~�h���E�F�A�w�b�_ Ver.2.30
 *	\author	Copyright(C) Arex Co. 2006-2011 All right Reserved.
 *	\author	������ЃA���b�N�X 2006-2011
 *	\date	2011/04/20 Ver.2.30
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#ifndef	_defSodiacAPI_H_
#define	_defSodiacAPI_H_

/*************************************************************************/ /**
 *	�������ʗ񋓌^
 **/ /*************************************************************************/
typedef enum _D_SODIAC_E
{
	D_SODIAC_E_OK = 0,			/**< ����I�� */
	D_SODIAC_E_DATA ,			/**< �f�[�^�ُ� */
	D_SODIAC_E_MEM ,			/**< �������ُ� */
	D_SODIAC_E_PAR ,			/**< �p�����[�^�ُ� */
	D_SODIAC_E_STATE ,			/**< ��Ԉُ� */
	D_SODIAC_E_NO_SUPPORT ,		/**< ���T�|�[�g�ُ� */
	D_SODIAC_E_OTHER			/**< ���̑��ُ� */
}	D_SODIAC_E;

/*************************************************************************/ /**
 *	�ʒm�񋓌^
 **/ /*************************************************************************/
typedef enum _D_SODIAC_ID
{
	D_SODIAC_ID_STOP = 0,		/**< �����f�[�^�Đ��I���ʒm */
	D_SODIAC_ID_NEXT,			/**< ���t���[�Y�w��\�ʒm */
	D_SODIAC_ID_ERROR,			/**< �ُ�ʒm */
	D_SODIAC_ID_PWMON,			/**< PWM�o�͊J�n�ʒm(�Đ��\�ʒm) */
	D_SODIAC_ID_PWMOFF,			/**< PWM�o�͒�~�ʒm */
	D_SODIAC_ID_MUTEOFF,		/**< MUTE�I�t�ʒm */
	D_SODIAC_ID_MUTEON			/**< MUTE�I���ʒm */
}	D_SODIAC_ID;

/*************************************************************************/ /**
 *	Sodiac bPause �ݒ�p�񋓌^
 **/ /*************************************************************************/
typedef enum _D_SODIAC_BOOL
{
	D_SODIAC_BOOL_TRUE  = 1,			/**< �^ */
	D_SODIAC_BOOL_FALSE = 0				/**< �U */
}	D_SODIAC_BOOL;

/*************************************************************************/ /**
 *	Sodiac Kind �ݒ�p�񋓌^
 **/ /*************************************************************************/
typedef	enum _D_SODIAC_KIND {
	D_SODIAC_KIND_DATA_OUT		= 0 ,	/**< �f�[�^�Đ� */
	D_SODIAC_KIND_PHRASE_OUT	= 1 ,	/**< �g�ݍ��킹�Đ� */
	D_SODIAC_KIND_SILENCE_OUT	= 2 ,	/**< �����Đ� */
	D_SODIAC_KIND_BEEP_OUT		= 3		/**< ���ʉ��Đ� */
}	D_SODIAC_KIND;

/*************************************************************************/ /**
 *	Sodiac Option �ݒ�p�񋓌^
 **/ /*************************************************************************/
typedef	enum _D_SODIAC_OPTION {
	D_SODIAC_OPTION_NONE       = 0x00 ,	/**< �I�v�V�������ݒ� */
	D_SODIAC_OPTION_PITCH_CONV = 0x01 ,	/**< �s�b�`�ϊ� */
	D_SODIAC_OPTION_SPEED_CONV = 0x02	/**< �X�s�[�h�ϊ� */
}	D_SODIAC_OPTION;

/*************************************************************************/ /**
 *	Null�|�C���^�ݒ�p�}�N��
 **/ /*************************************************************************/
#define	D_SODIAC_NULL	(0x00)

/*************************************************************************/ /**
 *	sodiac_get_version_info �̃p�����[�^�^
 **/ /*************************************************************************/
typedef struct _st_sodiac_version_info
{
	char	sodiac_version[8];	/**< Sodiac �̃o�[�W�����ԍ� */
	char	build_version[8];	/**< Build�ԍ� */
}	st_sodiac_version_info;

/*************************************************************************/ /**
 *	�Đ��p�p�����[�^�^
 **/ /*************************************************************************/
typedef struct _st_sodiac_param
{
	unsigned short	kind;		/**< D_SODIAC_KIND ����I�� */
	unsigned short	option;		/**< D_SODIAC_OPTION ���� OR�I�� */
	unsigned short	num;		/**< �Đ��ԍ��@�������E���ʉ�����ms */
	unsigned short	volume;		/**< 0(�ő�)�`32(����) */
	unsigned short	pitch;		/**< 100����Ƃ��đ傫���Ƃ��ɍ����������Ƃ��ɒႭ */
	unsigned short	speed;		/**< 100����Ƃ��đ傫���Ƃ��ɑ����������Ƃ��ɒx�� */
	unsigned short	ch;			/**< �Đ�����`�����l�� */
}	st_sodiac_param;

/*************************************************************************/ /**
 *	�R�[���o�b�N�|�C���^�^
 **/ /*************************************************************************/
typedef void (* fp_sodiac_callback)(int iCh, D_SODIAC_ID id, D_SODIAC_E err );

extern const char g_sodiac_copyright[];

#ifdef __cpluspluse
extern "C" {
#endif
/*************************************************************************/ /**
 *	�~�h���E�F�A�̏������ƃ^�C�}�[����N���b�N�̐ݒ�
 *
 *	\param[in]	ulTimerClock  : �^�C�}�[�ɋ��������̃N���b�N���g��(Hz)
 *	\retval		D_SODIAC_E_OK : ����ɓo�^���ꂽ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_init(unsigned long ulTimerClock);

/*************************************************************************/ /**
 *	�Đ���~�����荞�݂ɒʒm
 *
 *	\param[in]	usStartTime1 : PWM�I������A���v�p���[�I���܂ł̎���(mS)
 *	\param[in]	usStartTime2 : �A���v�p���[�I�����甭���܂ł̎���(mS)
 *	\param[in]	usEndTime    : �A���v�p���[�I�t����PWM��~�̎���(mS)
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_amp_ctrl(unsigned short usStartTime1, unsigned short usStartTime2, unsigned short usEndTime);

/*************************************************************************/ /**
 *	�Đ��̎��s
 *
 *	\param[in]	pParam        : �Đ�����t���[�Y���w�肷��
 *	\retval		D_SODIAC_E_OK : ����ɓo�^���ꂽ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_execute(st_sodiac_param *pParam);

/*************************************************************************/ /**
 *	�Đ����x�̐ݒ�
 *
 *	\param[in]	usSpeed       : �Đ����x ���ʂ�100�ő傫���قǑ���
 *	\retval		D_SODIAC_E_OK : ����ɓo�^���ꂽ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_speed_conv(unsigned short usCh, unsigned short usSpeed);

/*************************************************************************/ /**
 *	�Đ������̐ݒ�
 *
 *	\param[in]	usTone        : �Đ����� ���ʂ�100�ő傫���قǍ���
 *	\retval		D_SODIAC_E_OK : ����ɓo�^���ꂽ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_pitch_conv(unsigned short usCh, unsigned short usTone);

/*************************************************************************/ /**
 *	���ʂ̐ݒ�
 *
 *	\param[in]	usVolume      : ���� �ő傪0�ŁA�Œ��(C_SODIAC_VOLUMENUM-1)�̒l
 *	\retval		D_SODIAC_E_OK : ����ɓo�^���ꂽ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_volume_set(unsigned short usCh, unsigned short usVolume);

/*************************************************************************/ /**
 *	�Đ���~�����荞�݂ɒʒm
 *
 *	\retval		D_SODIAC_E_OK : ����ɓo�^���ꂽ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_stop(unsigned short usCh);

/*************************************************************************/ /**
 *	�Đ��̈ꎞ��~/�ĊJ
 *
 *	\param[in]	bPause : D_SODIAC_BOOL_TRUE�ňꎞ��~
 *						 D_SODIAC_BOOL_FALSE�ňꎞ��~����
 *	\retval		D_SODIAC_E_OK : ����ɓ��삵��
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_pause(unsigned short usCh, D_SODIAC_BOOL bPause);

/*************************************************************************/ /**
 *	�Đ��ł���t���[�Y�̐���Ԃ�
 *
 *	\return		�Đ��ł���t���[�Y�̐�
 **/ /*************************************************************************/
extern unsigned short sodiac_get_phrase_num(void);

/*************************************************************************/ /**
 *	Sodiac �~�h���E�F�A�̃o�[�W�������擾����
 *
 *	\param[out]	pInfo : �o�[�W���������i�[����̈�������|�C���^
 *	\retval		D_SODIAC_E_OK  : ����Ɋi�[���ꂽ
 *	\retval		D_SODIAC_E_PAR : �����ȃ|�C���^
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_get_version_info(st_sodiac_version_info *pInfo);

/*************************************************************************/ /**
 *	�C�x���g�t�b�N�֐��̓o�^
 *
 *	\param[in]	pEventHook    : �C�x���g�t�b�N�֐��ւ̃|�C���^
 *	\retval		D_SODIAC_E_OK : ����ɓo�^���ꂽ
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_regist_eventhook(fp_sodiac_callback pEventHook);

/*************************************************************************/ /**
 *	�Đ����擾
 *
 *	\param[out]	pState : !0 : �Đ��� / 0 : ��~��
 *	\retval		D_SODIAC_E_OK : ����Ɏ��s����
 **/ /*************************************************************************/
extern D_SODIAC_E sodiac_get_state(D_SODIAC_BOOL *pState);

#endif	/* _defSodiacAPI_H_ */
#ifdef __cpluspluse
}
#endif
/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex Co. Ltd, 2006-2011		  *
 **/ /*************************************************************************/
