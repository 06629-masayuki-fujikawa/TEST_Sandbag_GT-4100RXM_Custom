#ifndef	_SCRIPT_H_
#define	_SCRIPT_H_
//[]----------------------------------------------------------------------[]
///	@mainpage		
///	analayze user define/auto update script<br><br>
///	<b>Copyright(C) 2007 AMANO Corp.</b>
///	CREATE			2007/08/06 NISHIZATO<br>
///	UPDATE			
///	@file			script.h
///	@date			2007/08/06
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]

void	read_users(char *buff, long len);

void	au_init(void);

int		au_check_filename(uchar *name);
int		au_set_script(char *buff, ulong length);
int		au_get_cmd(struct _ft_sr_ *cmd);
void	au_get_change(char *change);
char	au_script_info_get(void);

#endif
