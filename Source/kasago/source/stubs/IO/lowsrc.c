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


/* ファイル番号 */

#define STDIN  0        /* 標準入力      （コンソール） */
#define STDOUT 1        /* 標準出力      （コンソール） */
#define STDERR 2        /* 標準エラー出力（コンソール） */

#define FLMIN  0               /* 最小のファイル番号 */
#define FLMAX  (IOSTREAM-1)    /* 最大のファイル番号 */

/* ファイルのフラグ */

#define O_RDONLY 0x0001     /* 読み込み専用 */
#define O_WRONLY 0x0002     /* 書き出し専用 */
#define O_RDWR   0x0004     /* 読み書き両用 */

/* 特殊文字コード */

#define CR 0x0d     /* 復帰     */
#define LF 0x0a     /* 改行     */

#define IOSTREAM 3

_UBYTE sml_buf[IOSTREAM];


/*************************************************************************/
/* 静的変数の定義：                                                      */
/* 低水準インタフェースルーチンで使用する静的変数の定義                  */
/*************************************************************************/
_SBYTE flmod[IOSTREAM];      /* オープンしたファイルのモード設定場所  */

/*************************************************************************/
/*  open:ファイルのオープン                                              */
/*        リターン値：ファイル番号（成功）                               */
/*                    -1          （失敗）                               */
/*************************************************************************/
_SINT open(_SBYTE *name,               /* ファイル名         */
     _SINT  mode,                      /* ファイルのモード   */
     _SINT  flg)                       /* 未使用             */
{
     return -1;

}

/*************************************************************************/
/*  close:ファイルのクローズ                                             */
/*       リターン値：0          （成功）                                 */
/*                   -1         （失敗）                                 */
/*************************************************************************/
_SINT close(_SINT fileno)              /* ファイル番号 */
{
     return -1;
}

/*************************************************************************/
/*  read:データの読み込み                                                */
/*        リターン値：実際に読み込んだ文字数 （成功）                    */
/*        		      -1 （失敗）                                        */
/*************************************************************************/
_SINT read(_SINT fileno,          /* ファイル番号           */
     _SBYTE *buf,                 /* 転送先バッファアドレス */
     _UINT count)                 /* 読み込み文字数         */
{
     return -1;
}
 
/*************************************************************************/
/*  write:データの書き出し                                               */
/*  	リターン値：実際に書き出した文字数  （成功）                     */
/*  		        -1  （失敗）                                         */
/*************************************************************************/
_SINT write(_SINT fileno,              /* ファイル番号           */
     _SBYTE *buf,                      /* 転送元バッファアドレス */
     _UINT count)                      /* 書き出し文字数         */
{
      return -1;

}

/******************************************************************************/
/*  lseek:ファイルの読み込み／書き出し位置の設定                              */
/*    リターン値：読み込み／書き出し位置のファイル先頭からのオフセット（成功）*/
/*                -1  （失敗）                                                */
/*      (コンソール入出力では、lseekはサポートしていません)                   */
/******************************************************************************/
_SDWORD lseek(_SINT fileno,            /* ファイル番号           */
       _SDWORD offset,                 /* 読み込み／書き出し位置 */
       _SINT base)                     /* オフセットの起点       */
{
       return -1L;
}

