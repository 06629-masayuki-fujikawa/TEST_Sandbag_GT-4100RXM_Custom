#ifndef	___NTNET_AUTO___
#define	___NTNET_AUTO___
/*[]----------------------------------------------------------------------[]
 *| System      : FT4800 ëº
 *| Module      : NT-NETé©ìÆëóêMêßå‰
 *[]----------------------------------------------------------------------[]
 *| Author      : MATSUSHITA
 *| Date        : 2007. 2. 8
 *| Update      :
 *[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/

void	ntautoInit(uchar clr);
void	ntautoTimeAdjust(void);
void	ntautoStartStop(ushort cond);
#define	ntautoStopReq()		ntautoStartStop(0)
#define	ntautoStartReq()	ntautoStartStop(1)
void	ntautoPostResult(uchar kind, uchar OKNG);
void	ntautoPostNearFull(ulong data);
extern	void	ntautoReqToSend(ulong data);
extern	void	ntautoSendCtrl(void);
void	ntautoSendCtrl_pcarsWeb(void);

#endif
