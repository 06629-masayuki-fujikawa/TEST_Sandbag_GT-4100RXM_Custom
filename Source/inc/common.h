#ifndef	___COMMONH___
#define	___COMMONH___
/*[]----------------------------------------------------------------------[]
 *|	filename: common.h
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ʔėp��`
 *| author	: machida.k
 *| date	: 2005.07.01
 *| update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#ifndef BOOL
#define	BOOL	uchar
#define	TRUE	1
#define	FALSE	0
#endif

// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#ifndef _countof
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#define	_countof(a)				(sizeof(a) / sizeof((a)[0]))
// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#endif
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#define	_offsetof(s,m)   (size_t)&(((s *)0)->m)
#define	_offset(cur, move, max)	(((cur) + (move) >= (max)) ? ((cur) + (move) - (max)) : ((cur) + (move)))
#define	_To2msVal(tm)			((tm) / 2 + 1)
#define	_To10msVal(tm)			((tm) / 10 + 1)
#define	_To20msVal(tm)			((tm) / 20 + 1)

#define	_MAKEWORD(c)			(ushort)(((ushort)(c)[0] << 8) + (c)[1])
#define	_MAKELONG(s1,s2)		(ulong)(((ulong)(s1) << 16) + (ulong)(s2))

#define		CMN_UNUSED_PARAMETER(v)	(void)(v)

#endif	/* ___COMMONH___ */

