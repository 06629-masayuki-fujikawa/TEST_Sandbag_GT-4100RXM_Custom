#ifndef	_OIBAN_H_
#define	_OIBAN_H_
//[]----------------------------------------------------------------------[]
///	@file		oiban.h
///	@brief		'oiban' management
/// @date		2008/09/22
///	@author		MATSUSHITA
///	@version	GT-7000_36
/// @note		í Çµí«Ç¢î‘ÅAí«Ç¢î‘îÕàÕëŒâû
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
extern	void	CountInit( uchar kind );
int		CountCheck( void );
extern	int		CountGet( uchar kind, ST_OIBAN *oiban );
extern	void	CountUp( uchar kind );
void	CountFree( uchar kind );
void	CountClear( uchar kind );
ulong	CountSel( ST_OIBAN *oiban );

void	CountAdd(SYUKEI *dst, SYUKEI *src);

#endif
