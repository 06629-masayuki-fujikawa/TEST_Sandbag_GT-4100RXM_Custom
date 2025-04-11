#include <string.h>
#include <stdio.h>
#include <stddef.h>

typedef signed char _SBYTE;
typedef unsigned char _UBYTE;
typedef signed short _SWORD;
typedef unsigned short _UWORD;
typedef signed int _SINT;
typedef unsigned int _UINT;
typedef signed long _SDWORD;
typedef unsigned long _UDWORD;

_SINT open(_SBYTE *name, _SINT mode, _SINT flg);
_SINT close(_SINT fileno);
_SINT read(_SINT fileno, _SBYTE *buf, _UINT count);
_SINT write(_SINT fileno, _SBYTE *buf, _UINT count);
_SDWORD lseek(_SINT fileno, _SDWORD offset, _SINT base);


/* �t�@�C���ԍ� */

#define STDIN  0        /* �W������      �i�R���\�[���j */
#define STDOUT 1        /* �W���o��      �i�R���\�[���j */
#define STDERR 2        /* �W���G���[�o�́i�R���\�[���j */

#define FLMIN  0               /* �ŏ��̃t�@�C���ԍ� */
#define FLMAX  (IOSTREAM-1)    /* �ő�̃t�@�C���ԍ� */

/* �t�@�C���̃t���O */

#define O_RDONLY 0x0001     /* �ǂݍ��ݐ�p */
#define O_WRONLY 0x0002     /* �����o����p */
#define O_RDWR   0x0004     /* �ǂݏ������p */

/* ���ꕶ���R�[�h */

#define CR 0x0d     /* ���A     */
#define LF 0x0a     /* ���s     */

#define IOSTREAM 3

_UBYTE sml_buf[IOSTREAM];


/*************************************************************************/
/* �ÓI�ϐ��̒�`�F                                                      */
/* �ᐅ���C���^�t�F�[�X���[�`���Ŏg�p����ÓI�ϐ��̒�`                  */
/*************************************************************************/
_SBYTE flmod[IOSTREAM];      /* �I�[�v�������t�@�C���̃��[�h�ݒ�ꏊ  */

/*************************************************************************/
/*  open:�t�@�C���̃I�[�v��                                              */
/*        ���^�[���l�F�t�@�C���ԍ��i�����j                               */
/*                    -1          �i���s�j                               */
/*************************************************************************/
_SINT open(_SBYTE *name,               /* �t�@�C����         */
     _SINT  mode,                      /* �t�@�C���̃��[�h   */
     _SINT  flg)                       /* ���g�p             */
{
     return -1;

}

/*************************************************************************/
/*  close:�t�@�C���̃N���[�Y                                             */
/*       ���^�[���l�F0          �i�����j                                 */
/*                   -1         �i���s�j                                 */
/*************************************************************************/
_SINT close(_SINT fileno)              /* �t�@�C���ԍ� */
{
     return -1;
}

/*************************************************************************/
/*  read:�f�[�^�̓ǂݍ���                                                */
/*        ���^�[���l�F���ۂɓǂݍ��񂾕����� �i�����j                    */
/*        		      -1 �i���s�j                                        */
/*************************************************************************/
_SINT read(_SINT fileno,          /* �t�@�C���ԍ�           */
     _SBYTE *buf,                 /* �]����o�b�t�@�A�h���X */
     _UINT count)                 /* �ǂݍ��ݕ�����         */
{
     return -1;
}
 
/*************************************************************************/
/*  write:�f�[�^�̏����o��                                               */
/*  	���^�[���l�F���ۂɏ����o����������  �i�����j                     */
/*  		        -1  �i���s�j                                         */
/*************************************************************************/
_SINT write(_SINT fileno,              /* �t�@�C���ԍ�           */
     _SBYTE *buf,                      /* �]�����o�b�t�@�A�h���X */
     _UINT count)                      /* �����o��������         */
{
      return -1;

}

/******************************************************************************/
/*  lseek:�t�@�C���̓ǂݍ��݁^�����o���ʒu�̐ݒ�                              */
/*    ���^�[���l�F�ǂݍ��݁^�����o���ʒu�̃t�@�C���擪����̃I�t�Z�b�g�i�����j*/
/*                -1  �i���s�j                                                */
/*      (�R���\�[�����o�͂ł́Alseek�̓T�|�[�g���Ă��܂���)                   */
/******************************************************************************/
_SDWORD lseek(_SINT fileno,            /* �t�@�C���ԍ�           */
       _SDWORD offset,                 /* �ǂݍ��݁^�����o���ʒu */
       _SINT base)                     /* �I�t�Z�b�g�̋N�_       */
{
       return -1L;
}

