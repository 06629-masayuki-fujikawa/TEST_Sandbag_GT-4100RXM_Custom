#ifndef	_FLASHDEF_H_
#define	_FLASHDEF_H_
//[]----------------------------------------------------------------------[]
///	@mainpage		
///	FLASH Memory Layout<br><br>
///	<b>Copyright(C) 2007 AMANO Corp.</b>
///	CREATE			2007/08/03 MATSUSHITA<br>
///	UPDATE			
///	@file			flashdef.h
///	@date			2007/08/03
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#include	"system.h"
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#include	"subtype.h"

//--- FLASH LAYOUT
#define	_FLASH2_BASE	0x00400000L
#define	_SUB_BOOT_OFS	0x0000E000L
#define	_SUB_APP_OFS	0x00080000L
#define	_SUB_APP_SIZ	0x00080000L
#define	GET_SBOOT()		(PRG_HDR*)(_FLASH2_BASE+_SUB_BOOT_OFS)
#define	GET_SAPP(n)		(PRG_HDR*)(_FLASH2_BASE+_SUB_APP_OFS+_SUB_APP_SIZ*(n))
#define	IsProgExist(p)	((p->proglen & 0xffff0000l) != 0xffff0000l)


// �ʐ؂�ւ����
#define SW_START_ADDR	_FLASH2_BASE			// �ʐ؂�ւ����A�h���X
#define SW_END_ADDR		(SW_START_ADDR+0x4000L)	// �ʐ؂�ւ����̏I�[+1
#define SW_RESERVE		(1024-8)				// �ʐ؂�ւ����̋󂫃G���A
#define SW_SECT_COUNT	((8*1024*2)/1024)		// �ʐ؂�ւ����̃Z�N�V������

typedef struct{
	uchar	flag;
	uchar	main;
	uchar	wave;
	uchar	param;
}SW_DATA;

#endif
