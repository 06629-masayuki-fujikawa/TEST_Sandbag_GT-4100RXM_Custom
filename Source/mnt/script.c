//[]----------------------------------------------------------------------[]
///	@mainpage		
///	analayze user define/auto update script<br><br>
///	<b>Copyright(C) 2007 AMANO Corp.</b>
///	CREATE			2007.08.06 NISHIZATO<br>
///	UPDATE			
///	@file			script.c
///	@date			2007.08.06
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#ifndef	WIN32
#include	"system.h"
#include	"mem_def.h"
#include	"prm_tbl.h"
#endif
#include	"ftpctrl.h"
#include	"remote_dl.h"
#include	"pri_def.h"
#include	"script.h"
#include	"AppServ.h"
#include	"fla_def.h"

extern	int stricmp(const char *d, const char *s);

enum {
	SECTION_NONE = -1,
	SECTION_TELNET,
	SECTION_FTPD,
	SECTION_FTPC
};

#define LINE_MAX	256
static	char	linebuff[LINE_MAX];
static	char	tmpbuff[32];

static	int		_read_line(char *buff, char *endp, char *line, int mode);
static	char	*_get_string(char *buff, char *dst, int maxlen, char sep);
#define	read_line(a,b,c)	_read_line(a,b,c,0)
#define	get_string(a,b,c)	_get_string(a,b,c,',')
static  uchar	script_extension_chk( struct _ft_sr_*, uchar );

//[]----------------------------------------------------------------------[]
///	@brief			read_line(read 1 line)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer pointer
///	@param[in]		endp	: end pointer
///	@param[out]		line	: read buffer
///	@param[in]		mode	: 0:skip all space, 1:keep one space
///	@return			read len:
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int _read_line(char *buff, char *endp, char *line, int mode)
{
	int		mode0 = mode;
	char	*p, *q;
	int		len;
	char	*p_work;
	p = buff;
// try read valid line
	do {
		if (p >= endp)
			return -1;		// EOF
		len = 0;
		q = line;
		do {
			if (*p == '\n') {
			// end of line
				p++;
				break;
			}
			if (*p < ' ' || (*p == ' ' && mode0 == 0)) {
			// skip space & control code
				p++;
			}
			else {
				if (*p == ' ')
					mode0 = 0;			// omitte following space
				else
					mode0 = mode;		// copy next space
			// copy data
				*q++ = *p++;
				len++;
				if (len > LINE_MAX) {
				// if overflow, skip this line
					while(p < endp) {
						if (*p == '\n') {
							p++;
							break;
						}
						p++;
					}
					len = 0;
					break;
				}
			}
		} while(p < endp);
	} while (len == 0);
	line[len] = '\0';
#ifdef	WIN32
	printf(">%s\n", line);
#endif
	p_work = (char*)(p - buff);

	return (int)(p_work);
}

//[]----------------------------------------------------------------------[]
///	@brief			get_string(get string parameter)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer pointer
///	@param[out]		dst		: destination
///	@param[in]		maxlen	: max length
///	@param[in]		sep		: separator
///	@return			src		: new pointer
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	char *_get_string(char *buff, char *dst, int maxlen, char sep)
{
	char	*src;
	int 	len;

	src = buff;
	maxlen--;
	len = 0;
	while(*src == ' ')
		src++;		// remove leading space
	while(*src) {
		if (*src == sep) {
			src++;
			break;
		}
		if (len < maxlen) {
			*dst++ = *src;
			len++;
		}
		src++;
	}
	while(len) {
		dst--;		// remove tail space
		if (*dst != ' ') {
			dst++;
			break;
		}
		len--;
	}
	*dst = '\0';
	return src;
}

//[]----------------------------------------------------------------------[]
///	@brief			read_userdefinition(sub CPU boot is failed)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@param[in]		len		:	
///	@return			void
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	2007.11.30 セクション定義修正
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	read_users(char *buff, long len)
{
}

/*--------------------------------------------------------------------------*/
/*		AUTO UPDATE															*/
/*--------------------------------------------------------------------------*/
static	struct _au_ctr {
		char	*buff;
		char	*endp;
		char	chg[2][3];
		char	script;				// スクリプトファイル解析実行フラグ
} au_ctr;

uchar	workbuff[508];// 暫定的に定義してあります

extern	const uchar	SYSMNT_PARAM_FILENAME[];

//[]----------------------------------------------------------------------[]
///	@brief			au_init(auto update init)
//[]----------------------------------------------------------------------[]
///	@return			void
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	au_init(void)
{
	memset(&au_ctr, 0, sizeof(au_ctr));
}

//[]----------------------------------------------------------------------[]
///	@brief			au_check_filename(check auto update script file name)
//[]----------------------------------------------------------------------[]
///	@param[in]		name	:	
///	@return			1/0		: OK(1)/  (0)
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
int		au_check_filename(uchar *name)
{
	return (stricmp((const char*)name, remotedl_script_get()) == 0)? 1 : 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			au_set_script(set auto update script)
//[]----------------------------------------------------------------------[]
///	@param[in]		buff	: buffer
///	@param[in]		length	: length
///	@return			1/0		: OK(1)/  (0)
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
int		au_set_script(char *buff, ulong length)
{
	if (length == 0 || length >= AUTO_SCRIPT_MAX) {
		au_ctr.buff = SCR_buff;
		au_ctr.endp = au_ctr.buff;
		return 0;
	}
	memcpy(SCR_buff, buff, length);
	au_ctr.buff = SCR_buff;
	au_ctr.endp = au_ctr.buff + length;
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			modify_filename(fix-up substitution characters)
//[]----------------------------------------------------------------------[]
///	@param[in]		dst		:	
///	@param[in]		src		:	
///	@param[in]		max		:	
///	@return			1/0		: OK(1)/  (0)
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
static	int		modify_filename(char *dst, char *src, int max)
{
	char	*endp = dst + max;
	while(*src) {
		if (*src != '?') {
			if (dst >= endp)
				return 0;
			*dst++ = *src++;
		}
		else {
			src++;
			if (*src == 'D') {
				if ((dst+8) >= endp)
					return 0;
#ifndef	WIN32
				intoas((uchar*)dst, CLK_REC.year, 4);
				dst += 4;
				intoas((uchar*)dst, (ushort)CLK_REC.mont, 2);
				dst += 2;
				intoas((uchar*)dst, (ushort)CLK_REC.date, 2);
				dst += 2;
#else
				sprintf(dst, "%04d%02d%02d", 2007, 10, 16);
				dst += 8;
#endif
			}
			else if (*src == 'T') {
				if ((dst+6) >= endp)
					return 0;
#ifndef	WIN32
				intoas((uchar*)dst, (ushort)CLK_REC.hour, 2);
				dst += 2;
				intoas((uchar*)dst, (ushort)CLK_REC.minu, 2);
				dst += 2;
				intoas((uchar*)dst, (ushort)CLK_REC.seco, 2);
				dst += 2;
#else
				sprintf(dst, "%02d%02d%02d", 13, 14, 33);
				dst += 6;
#endif
			}
			else if (*src == 'P') {
				if ((dst+4) >= endp)
					return 0;
#ifndef	WIN32
				intoasl((uchar*)dst, (ulong)CPrmSS[S_SYS][1], 4);
				dst += 4;
#else
				sprintf(dst, "%04d", 777);
				dst += 4;
#endif
			}
			else if (*src == 'I') {										// デバイスIDをセットする
				if ((dst+6) >= endp){
					return 0;
				}
				sprintf(dst, "%06d", 0);
				dst += 6;
			}
			else if (*src == 'C') {										// 駐車場名をセットする
				uchar	i;
				// 駐車場名はHeaderから取得する
				if (_FLT_RtnKind(Header_Rsts) == FLT_NODATA) {			// ヘッダ情報がない場合はエラーとする
					return 0;
				}
				if ((dst+sizeof(Header_Data[0])) >= endp){
					return 0;
				}
				for( i=0;i<sizeof(Header_Data[0]);i++ ){
					if( Header_Data[0][i] != 0x20 && Header_Data[0][i] != 0 ){
						*dst = Header_Data[0][i];
						dst++;
					}
				}
			}
			else if (*src == 'M') {										// 機械Noをセットする
				if ((dst+2) >= endp){
					return 0;
				}
				intoasl((uchar*)dst, (ulong)CPrmSS[S_PAY][2], 2);
				dst += 2;
			}
			else {
				return 0;
			}
			src++;
		}
	}
	if (dst >= endp)
		return 0;
	*dst = '\0';
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			au_get_cmd(get auto update command)
//[]----------------------------------------------------------------------[]
///	@param[in]		cmd		: struct _ft_sr_
///	@return			0/1/-1	: 0(EOF)<br>
///							  1(VALID COMMAND)<br>
/// 						  -1(INVALID)
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
int		au_get_cmd(struct _ft_sr_ *cmd)
{
	memset(cmd, 0, sizeof(*cmd));
	if (au_ctr.buff == NULL) {
		SetSnd_RecvStatus( FCMD_REQ_RECV );
		MakeRemoteFileName( cmd->remote, cmd->local, 0 );
		au_ctr.script = 0;
		set_script_file_name( cmd->local );
		return 1;
	}
	else {
	// exec script
		int		w, type;
		char	*p, *p2;

		for (w = 0; w < 3; w++) {
			au_ctr.chg[1][w] |= au_ctr.chg[0][w];			// = M/S/P selecting
			au_ctr.chg[0][w] = 0;
		}
		p = linebuff;
		do {
			w = read_line(au_ctr.buff, au_ctr.endp, p);			// get 1 line
			au_ctr.buff += w;
			if ( w < 0 ){								// ファイルの終了
				if( remotedl_status_get() == R_DL_EXEC ){
					if( au_ctr.script ){				// 有効行が１行でもある場合
						return 0;						// detect EOF
					}else{								// １行も有効スクリプト行がない場合
						return -1;						// Script Err
					}
				}else{
					return 0;							// detect EOF
				}
			}
		} while(*p == '#');
		au_ctr.script++;								// スクリプト解析実行
		p = get_string(p, tmpbuff, sizeof(tmpbuff));			// get command
		if (strlen(tmpbuff) != 2)
			return -1;		// invalid command
		if (toupper(tmpbuff[1]) == 'U') {
			SetSnd_RecvStatus( FCMD_REQ_SEND );
		}
		else if (toupper(tmpbuff[1]) == 'D') {
			SetSnd_RecvStatus( FCMD_REQ_RECV );
		}
		else {
			return -1;		// invalid command
		}
		if (toupper(tmpbuff[0]) ==  'F') {
		// Fx : file up/down
			p = get_string(p, (char*)workbuff, sizeof(cmd->remote));		// get remote file
			if (! modify_filename((char*)cmd->remote, (char*)workbuff, (int)sizeof(cmd->remote)))
				return -1;
			p = get_string(p, (char*)cmd->local, sizeof(cmd->local));		// get local file
			return 1;	// OK
		}
		else if (toupper(tmpbuff[0]) ==  'P') {
		// Px : program or parameter up/down
			p = get_string(p, (char*)workbuff, sizeof(cmd->remote));		// get remote file
			if (! modify_filename((char*)cmd->remote, (char*)workbuff, sizeof(cmd->remote)))
				return -1;
			p = get_string(p, tmpbuff, sizeof(tmpbuff));			// get local target
			if (strlen(tmpbuff) != 2)
				return -1;	// invalid local target
			if (toupper(tmpbuff[0]) == 'A') {
				strcpy((char*)cmd->local, "/SYS/ACT/");
			}
			else if (toupper(tmpbuff[0]) == 'S') {
				strcpy((char*)cmd->local, "/SYS/STBY/");
			}
			else {
				return -1;	// invalid local target
			}
			if (toupper(tmpbuff[1]) == 'P') {
			// parameter
				type = 2;
				if( script_extension_chk( cmd, 'P' ) ){
					return -1;
				}
				strcat((char*)cmd->local, (char*)SYSMNT_PARAM_FILENAME);
			}
			else {
				if (toupper(tmpbuff[1]) == 'M') {
				// main
					type = 0;
					if( script_extension_chk( cmd, 'M' ) ){
						return -1;
					}
					strcat((char*)cmd->local, "MAIN");
				}
				else if (toupper(tmpbuff[1]) == 'V') {
				// wave
					type = 1;
					if( script_extension_chk( cmd, 'V' ) ){
						return -1;
					}
					strcat((char*)cmd->local, "VOICE");
				}
				else {
					return -1;	// invalid local target
				}
			// copy remote file name to local
				p2 = (char*)&cmd->remote[strlen((char*)cmd->remote)];
				do {
					if (p2 <= (char*)cmd->remote)
						break;
					p2--;
				} while(*p2 != '/');
				w = (int)strlen(p2);
				if (w <= 1)
					return -1;		// invalid file name
				w += (int)strlen((char*)cmd->local);
				if (w > sizeof(cmd->local))
					return -1;		// invalid file name
				strcat((char*)cmd->local, p2);
			}
			if (cmd->code == FCMD_REQ_RECV) {
				if (cmd->local[5] == 'A')
					return -1;	// cannot write into ACT dir.
			// check it needs to change surface or not
				p = get_string(p, tmpbuff, sizeof(tmpbuff));
				if (tmpbuff[0] != '\0') {
					if (toupper(tmpbuff[0]) == 'Y') {
							if (tmpbuff[1] != '\0'){
								return -1;		// invalid
							}
							au_ctr.chg[0][type] = 1;
					}else{
						if( remotedl_status_get() == R_DL_EXEC ){
							return -1;		// invalid								// 指定文字列以外はエラーとする
						}
					}
				}else{
					if( remotedl_status_get() == R_DL_EXEC ){
						return -1;		// invalid									// 更新指定がない場合はエラーとする
					}
				}
			}
			return 1;	// OK
		}
	}
	return -1;
}

//[]----------------------------------------------------------------------[]
///	@brief			au_get_change(get surface change information)
//[]----------------------------------------------------------------------[]
///	@param[in]		change	:	
///	@return			void
///	@attention		None
///	@author			NISHIZATO
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2007.08.06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]
void	au_get_change(char *change)
{
	memcpy(change, au_ctr.chg[1], 3);
}

//[]----------------------------------------------------------------------[]
///	@brief			au_script_info_get
//[]----------------------------------------------------------------------[]
///	@return			char
///	@attention		void
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010.10.31<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
char	au_script_info_get(void)
{
	return(au_ctr.script);
}

/*[]----------------------------------------------------------------------[]*/
///	@brief			スクリプトファイル拡張子チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		*p		: ファイル情報ポインタ
///	@param[in]		type	: ターゲット指定
///	@return			ret		: 0：拡張子一致 1：拡張子不一致
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2010-10-20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]
const char extension_table[2][3]={
	{'B','I','N'}
,	{'C','S','V'}
};
uchar	script_extension_chk( struct _ft_sr_ *p, uchar type )
{
	ushort i,index;
	uchar w_type = 0;
	uchar ret = 0;
	
	index = 0;
	if( type == 'P' ){
		w_type = 1;
	}
	
	for( i=0; (i<sizeof(p->remote) && p->remote[i] != 0); i++ ){
		if( p->remote[i] == 0x2e ){							// .(ピリオド)発見
			index = i+1;									// 次のデータから比較対象とする
		}
	}

	if( !index ){											// ピリオドがない
		ret = 1;											// NGとする
	}else{
		for( i=0; i<sizeof(extension_table[0]); i++ ){
			if( toupper(p->remote[index+i]) != extension_table[w_type][i] ){
				ret = 1;
				break;
			}
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*/
///	@brief			スクリプトファイルからファイルシステムのファイル名を作成
//[]----------------------------------------------------------------------[]
///	@param[in]		*p			: ファイルシステムポインタ
///	@param[in]		remote_path	: リモートファイル名
///	@return			void
///	@attention		None
///	@author			Namioka
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012-05-04<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	get_script_file_name(  char* p, const char* remote_path )
{
	short i,j,copy_start,invalid_line;
	
	invalid_line = 0;
	memset( linebuff, 0, sizeof( linebuff ));
	memset( tmpbuff, 0, sizeof( tmpbuff ));
	
	if( GetFTPMode() == _FTP_CLIENT ){
		if( SCR_buff[0] != 0 ){
			for( i=0,j=0,copy_start=0; i<sizeof(SCR_buff); i++ ){
				if( SCR_buff[i] == '#' ){
					invalid_line = 1;
				}
				if( !invalid_line){
					if( SCR_buff[i] == ',' && copy_start == 0 ){
						copy_start = 1;
						i++;
					}else{
						if( copy_start ){
							if( SCR_buff[i] == ','){
								copy_start = 2;
							}
						}
					}
					if( copy_start == 1 ){
						linebuff[j++] = SCR_buff[i];
					}else if( copy_start == 2 ){
						break;
					}
				}else{
					if( SCR_buff[i] == '\n' ){
						invalid_line = 0;
					}
				}
			}
			for( i = 0; i<sizeof(linebuff) && linebuff[i]; i++ ){
				if( linebuff[i] == '.' ){
					for( j = i; ; j-- ){
						if( linebuff[j] == '/' ){
							strcpy( tmpbuff,&linebuff[j+1] );
							i = sizeof(linebuff);
							break;
						}
					}
				}
			}
			memset( linebuff, 0, sizeof( linebuff ));
			memcpy( linebuff, tmpbuff, sizeof( tmpbuff ));
			modify_filename( tmpbuff, linebuff, sizeof( tmpbuff ));
			if( tmpbuff[0] != 0 ){
				strcat( p, tmpbuff);
			}
		}
	}else{
		strcpy( p, remote_path );
	}
}

