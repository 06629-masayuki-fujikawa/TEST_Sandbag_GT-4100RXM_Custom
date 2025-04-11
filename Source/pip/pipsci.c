/***********************************************************************/
/*																	   */
/*	FILE		:SCI.c												   */
/*	DATE		: Aug 23, 2000										   */
/*	DESCRIPTION :Main Program										   */
/*	CPU TYPE	:H8S/2357F											   */
/*																	   */
/***********************************************************************/

#define GLOBAL extern

#include	<ctype.h>
#include	"iodefine.h"
#include	"system.h"
#include	"pip_def.h"

/********************* FUNCTION DEFINE **************************/
void sci_init_h(void);
void sci_tx_start_h(void);

/* �g�n�r�s���V���A���h�^�e�N�� */
void sci_init_h(void)
{
	SCI10.SCR.BYTE = 0x70;			// ��M���싖��
	dummy_Read = SCI10.SSR.BYTE;
	SCI10.SSR.BYTE = 0xC4;			// �G���[�t���O�N���A
}

/* �g�n�r�s���V���A���h�^�e���M�J�n */
void sci_tx_start_h(void)
{
	SCI10.SCR.BIT.TE = 1;			// �V���A�����M���싖��
	SCI10.TDR = PIPRAM.huc_txdata_i_h[PIPRAM.ui_txpointer_h];		// �P�o�C�g���M
	PIPRAM.ui_txpointer_h++;
	dummy_Read = SCI10.SSR.BYTE;	// Dummy Read
	PIPRAM.uc_send_end_h = 0;		// ���M�����t���O�N���A

	SCI10.SCR.BIT.TIE = 1;			// �f�[�^�G���v�e�B(TIE)���荞�݋���
}

/* �g�n�r�s���V���A���h�^�e��~ */
void sci_stop_h(void)
{
	_di();
	SCI10.SCR.BIT.TIE  = 0;			// ���M�f�[�^�G���v�e�B(TXI)���荞�݋֎~ 
	SCI10.SCR.BIT.RIE  = 0;			// ��M�����݋֎~ 
	SCI10.SCR.BIT.TE   = 0;			// ���M����֎~ 
	SCI10.SCR.BIT.RE   = 0;			// ��M����֎~ 
	SCI10.SCR.BIT.TEIE = 0;			// ���M�I��(TEI)���荞�݋֎~ 
	dummy_Read = SCI10.SSR.BYTE;	// Dummy Read
	_ei();

}

