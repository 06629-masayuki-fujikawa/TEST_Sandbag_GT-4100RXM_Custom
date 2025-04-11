//[]----------------------------------------------------------------------[]
///	@file		FlashSerial_RSPI.c
///	@brief		�V���A���t���b�V��ROM SPI�h���C�o
/// @date		2012/02/09
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include "iodefine.h"
#include "system.h"
#include "FlashSerial.h"
/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void Rspi_from_Init(void);
void Rspi_from_SPRI( void );
unsigned char Rspi_from_SndReq( unsigned short Length );
unsigned char Rspi_from_IsSndRcvCmp( void );

/*--- Define --------------------*/

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			�V���A���t���b�V��ROM SPI�h���C�o������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Rspi_from_Init(void)
{
	// ��M�Ǘ�����������
	Rspi_RcvCtrl.RcvReqCnt  = 0;
	Rspi_RcvCtrl.RcvCmpCnt  = 0;
	Rspi_RcvCtrl.RcvCmpFlg  = 1;

	// ���M�Ǘ�����������
	Rspi_SndCtrl.SndReqCnt = 0;
	Rspi_SndCtrl.SndCmpCnt = 0;
	Rspi_SndCtrl.ReadIndex = 0;
	Rspi_SndCtrl.SndCmpFlg = 1;

	memset(Rspi_RcvBuf,0xFF,sizeof(Rspi_RcvBuf));
	memset(Rspi_SndBuf,0xFF,sizeof(Rspi_SndBuf));
	From_Access_SemFree(0);		// FROM�A�N�Z�X���͊J�����Ă���

	// RSPI0�̏����ݒ�
	MSTP(RSPI0) = 0;
	RSPI0.SSLP.BYTE = 0x00;		// SSL0P�`SSL3P��L�A�N�e�B�u(�f�t�H���g)
	RSPI0.SPPCR.BYTE = 0x00;	// (�f�t�H���g)
	RSPI0.SPSCR.BYTE = 0x00;	// (�f�t�H���g)
	RSPI0.SPBR.BYTE = 0x01;		// 12.5Mbps
	RSPI0.SPDCR.BYTE = 0x20;	// 32-bit data, 1 frame
	RSPI0.SPCKD.BYTE = 0x00;	// (�f�t�H���g)
	RSPI0.SSLND.BYTE = 0x00;	// (�f�t�H���g)
	RSPI0.SPND.BYTE = 0x00;		// (�f�t�H���g)
	// SPCMD��SSL�M���ێ��r�b�g�͓]������f�[�^���ɂ���ĕύX����K�v������܂��B(�����l�͓]���I���Ńl�Q�[�g�Ƃ���)
	RSPI0.SPCMD0.WORD = 0x2703;	// b15:SCKDEN=0(RSPCK�x����1RSPCLK)
								// b14:SLNDEN=0(SSL�l�Q�[�g�x����1RSPCK)
								// b13:SPNDEN=1(���A�N�Z�X�x����RSPI���A�N�Z�X�x�����W�X�^�iSPND�j�̐ݒ�l)
								// b12:LSBF=0(MSB�t�@�[�X�g)
								// b11-8:SPB=111(8bit)
								// b7:SSL�M�����x���ێ��r�b�gSSLKP=0(�]���I�����ɑSSSL�M�����l�Q�[�g)
								// b6-4:SSLA=000(SSL0)
								// b3-2:BRDV=00(�x�[�X�̃r�b�g���[�g��I��)
								// b1:CPOL=1(�A�C�h������RSPCK���g1�h)
								// b0:CHPA=1(��G�b�W�Ńf�[�^�ω��A�����G�b�W�Ńf�[�^�T���v��)
								// ���[�h3

	RSPI0.SPCR.BYTE = 0x48;		// Enable RSPI in master mode

}

//[]----------------------------------------------------------------------[]
///	@brief			�V���A���t���b�V��ROM SPI��M���荞�ݏ���
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			
/// �N���b�N�����̂��ߑ��M�Ǝ�M�̓Z�b�g�ƂȂ��Ă���B
/// ���M�^��M�̊��荞�݂�ʁX�ɐ��䂷��ƃI�[�o�[��������������\��������
/// ��M���荞�݂Ŏ�M�f�[�^����荞�񂾂��ƂŁA���M�������s�Ȃ��Γ]�����x�͎኱�x��
/// (��90��)�Ȃ邪�I�[�o�[�����G���[�͔������Ȃ��B
/// ���̂��߂Ɏ�M���荞�ݏ����݂̂ő���M�̗����̐�����s��
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Rspi_from_SPRI( void )
{

// ��M����
	Rspi_RcvBuf[Rspi_RcvCtrl.RcvCmpCnt] = (unsigned char) (RSPI0.SPDR & 0xFF) ;	// ��M�o�b�t�@�֓]����A��M�f�[�^���J�E���g�A�b�v
	Rspi_RcvCtrl.RcvCmpCnt++;
	if (Rspi_RcvCtrl.RcvReqCnt <= Rspi_RcvCtrl.RcvCmpCnt) {
		RSPI0.SPCR.BIT.SPRIE = 0;												// RSPI��M���荞�݋֎~
		Rspi_RcvCtrl.RcvCmpFlg = 1;												// ��M��������(�����t���O��ON)
	}
// ���M����
	Rspi_SndCtrl.SndCmpCnt++;													// ���M�����f�[�^���J�E���g�i�A�b�v�j
	if (Rspi_SndCtrl.SndReqCnt > Rspi_SndCtrl.SndCmpCnt) {						// ���M�f�[�^�L
		if(Rspi_SndCtrl.SndReqCnt == (Rspi_SndCtrl.SndCmpCnt + 1)){				// �]���̍Ō�̎�
			RSPI0.SPCMD0.WORD = 0x2703;// SSL�M���͓]�������ŉ���
		}
		RSPI0.SPDR = Rspi_SndBuf[ Rspi_SndCtrl.ReadIndex++];					// ���M�o�b�t�@����]�� 
	}
	else {
		Rspi_SndCtrl.SndCmpFlg = 1;												// ���M��������(�����t���O��ON)
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			�V���A���t���b�V��ROM SPI���M�v������
//[]----------------------------------------------------------------------[]
///	@param[in]		Length	: �]���f�[�^��1�`RSPI_BUF_SIZE
///	@return			0		: �ُ�I��
///					1		: ����I��
///	@author			A.iiizumi
///	@note			���M�o�b�t�@Rspi_SndBuf[]�Ƀf�[�^���Z�b�g���Ă���R�[�����邱��
///					��M�����f�[�^��Rspi_RcvBuf[]�Ɋi�[����܂�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char Rspi_from_SndReq( unsigned short Length )
{
	if ((Length == 0)||(Length > RSPI_BUF_SIZE)) {// �Z�[�t�e�B�@�\ 
		Rspi_SndCtrl.SndCmpFlg = 1;				// ���M��������(�����t���O��ON)
		Rspi_RcvCtrl.RcvCmpFlg = 1;				// ��M��������(�����t���O��ON)
		return (unsigned char)0;				// �ُ�I���i���M�J�n�ł��Ȃ������j
	}

	if( 0 == RSPI0.SPSR.BIT.IDLNF ){			// SPI0���A�C�h�����
		Rspi_SndCtrl.SndCmpFlg = 0;				// ���M�J�n(�����t���O��OFF)
		Rspi_SndCtrl.SndReqCnt = Length;		// ���M�v���f�[�^�����Z�b�g
		Rspi_SndCtrl.SndCmpCnt = 0;				// ���M�����f�[�^�������Z�b�g
		Rspi_SndCtrl.ReadIndex = 1;				// �ŏ��̂P�����]��(�C���f�b�N�X�̃Z�b�g)

		Rspi_RcvCtrl.RcvCmpFlg = 0;				// ���M�J�n(�����t���O��OFF)
		Rspi_RcvCtrl.RcvReqCnt = Length;		// ���M�v���f�[�^�����Z�b�g
		Rspi_RcvCtrl.RcvCmpCnt = 0;				// ���M�����f�[�^�������Z�b�g
		if(Length == 1){// �]����1�o�C�g�̎�
			RSPI0.SPCMD0.WORD = 0x2703;			// �]���̍Ō��� SSL�M���͓]�������ŉ���
		}else {
			RSPI0.SPCMD0.WORD = 0x2783;			// �]���̓r����� SSL�M���̓A�N�e�B�u��ێ�
		}
		// NOTE:�K�����荞�݋������Ă���SPDR�Ƀf�[�^���Z�b�g���邱��(���荞�݂���肱�ڂ�����)
		RSPI0.SPCR.BIT.SPRIE = 1;				// RSPI��M���荞�݋���
		RSPI0.SPDR = Rspi_SndBuf[0];			// �ŏ��̂P�����𑗐M�o�b�t�@����]��
		return (unsigned char)1;				// ����I���i���M�J�n�ł����j
	}
	else {										// SPI����M���쒆
		Rspi_SndCtrl.SndCmpFlg = 1;				// ���M��������(�����t���O��ON)
		Rspi_RcvCtrl.RcvCmpFlg = 1;				// ��M��������(�����t���O��ON)
		RSPI0.SPCR.BIT.SPRIE = 0;				// RSPI��M���荞�݋֎~
		return (unsigned char)0;				// �ُ�I���i���M�J�n�ł��Ȃ������j
	}
	return (unsigned char)1;
}
//[]----------------------------------------------------------------------[]
///	@brief			�V���A���t���b�V��ROM SPI����M��������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0		: ����M������
///					1		: ����M����
///	@author			A.iiizumi
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char	Rspi_from_IsSndRcvCmp( void )
{
	if((1 == Rspi_RcvCtrl.RcvCmpFlg)&&
	   (1 == Rspi_SndCtrl.SndCmpFlg)){
		return (unsigned char)1;
	}
	return (unsigned char)0;
}
