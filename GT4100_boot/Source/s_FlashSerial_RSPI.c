//[]----------------------------------------------------------------------[]
///	@file		s_FlashSerial_RSPI.c
///	@brief		�u�[�g�v���O�����p �V���A���t���b�V��ROM SPI�h���C�o
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include "iodefine.h"
#include "s_FlashSerial.h"
/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void Rspi_from_Init(void);
static void spi_wait(void);
unsigned char FlashReadData_direct(const unsigned long , unsigned char *, const unsigned long );

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

#pragma section F_UPDATE_FUNC	// �v���O�����������ݎ��v���O���������RAM�ɃR�s�[����̈�

//[]----------------------------------------------------------------------[]
///	@brief			�\�t�g�E�F�A�E�F�C�g SPI�p
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void spi_wait(void)
{
	unsigned int count = 5;
	while(count > 0) {
		count--;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			���[�h�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: ���[�h�J�n�J�n�A�h���X
///	@param[out]		*data	: ���[�h�f�[�^���������ރ|�C���^
///	@param[in]		size	: ���[�h�f�[�^�T�C�Y(1�`4096(1�Z�N�^))
///	@return			0		: �ُ�I��
///					1		: ����I��
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadData_direct(const unsigned long address, unsigned char *data, const unsigned long size)
{
	unsigned char	command[4];
	unsigned long i ;
	volatile unsigned char	dmmy;

	command[0] = 0x3;						// Read
	command[1] = (unsigned char)(address >> 16);
	command[2] = (unsigned char)(address >>  8);
	command[3] = (unsigned char)(address >>  0);

	// �R�}���h���M�t�F�[�Y
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// �]���̓r���̎� SSL�M���̓A�N�e�B�u��ێ�
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
	for (i = 0; i < 4; i++)
	{
		RSPI0.SPDR = command[i];
		while (RSPI0.SPSR.BIT.IDLNF);
		spi_wait();							// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
		dmmy = (unsigned char) RSPI0.SPDR;	// �_�~�[���[�h
	}
	// �f�[�^��M�t�F�[�Y
	for (i = 0; i < size; i++) {
		if(i == (size-1)) {
			RSPI0.SPCMD0.WORD = 0x2703;		// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
		}
		RSPI0.SPDR = 0xFF;
		/* Wait for transfer to complete */
		while (RSPI0.SPSR.BIT.IDLNF) ;		// ensure transmit register is empty
		spi_wait();							// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
		data[i] = (unsigned char) (RSPI0.SPDR & 0xFF) ;
	}
	return (unsigned char)1;				// ����I��
}

