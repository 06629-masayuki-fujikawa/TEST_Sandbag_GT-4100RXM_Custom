//[]----------------------------------------------------------------------[]
///	@file		FlashSerial_RSPI.c
///	@brief		�V���A���t���b�V��ROM �A�N�Z�XAPI
/// @date		2012/02/09
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*--- Include -------------------*/
#include "iodefine.h"
#include "system.h"
#include "FlashSerial.h"
#include "mdl_def.h"
/*--- Pragma --------------------*/

/*--- Prototype -----------------*/
void SetRsten(void);
void SetRst(void);
unsigned char FlashReadStatus(void);
void FlashWriteStatus( unsigned char );
unsigned char  FlashErase ( const unsigned long, const unsigned char );
unsigned char  FlashReadData ( const unsigned long, unsigned char *, const unsigned long );
void FlashWriteData ( unsigned long, unsigned char const *);
void FlashJEDECReadId ( unsigned char * );

unsigned char FlashReadStatus_direct(void);
void FlashWriteStatus_direct(unsigned char );
unsigned char FlashErase_direct(const unsigned long , const unsigned char );
void FlashWriteData_direct(unsigned long , unsigned char const *);
unsigned char FlashReadData_direct(const unsigned long , unsigned char *, const unsigned long );
void FlashJEDECReadId_direct(unsigned char *);
unsigned char From_Access_SemGet(unsigned char FlashSodiac);
void From_Access_SemFree(unsigned char FlashSodiac);
static void spi_wait(void);
static void WriteEnable(void);
static void WriteEnable_direct(void);
static unsigned char	f_from_access_sem;			// SPI FROM�A�N�Z�X�t���O 0:�A�N�Z�X���J����� 1:�A�N�Z�X���擾���

extern		unsigned char			Sodiac_FROM_Get;
extern		char use_avm;

/*--- Define --------------------*/

/*--- Gloval Value --------------*/

/*--- Extern --------------------*/
//[]----------------------------------------------------------------------[]
///	@brief			SPI FROM�A�N�Z�X���擾�i�Z�}�t�H�擾�j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			0: �A�N�Z�X���擾���s
///					1: �A�N�Z�X���擾����
///	@author			A.iiizumi
///	@attention		SPI FROM�ɃA�N�Z�X����O�Ƀt���O���擾����B
///					�擾�ł��Ȃ��ꍇ�̓^�X�N�؂芷�����s���Ȃ���擾�\�ƂȂ�܂ő҂��ƁB
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char From_Access_SemGet(unsigned char FlashSodiac)
{	//	0/1/2 = none/Flash/Sodiac
	if(FlashSodiac == 1){
		/* ch0, ch1�ǂ��炩�Đ����̏ꍇ	*/
		if(use_avm == AVM_CONNECT){												// AVM�ڑ��L��
			if( (0 == AVM_Sodiac_Ctrl[0].play_message_cmp) || ( 0 == AVM_Sodiac_Ctrl[1].play_message_cmp) )
			{
				return(unsigned char)0;// �擾���s
			}
		}
	}
	if(FlashSodiac == 2){
		if(Rspi_from_IsSndRcvCmp() == 0){										// RSPI����M��
			return(unsigned char)0;			// �擾���s
		}
	}
	if(f_from_access_sem == 0){ // from�ɃA�N�Z�X���Ă��Ȃ�
		f_from_access_sem = 1; // �A�N�Z�X�t���O�Z�b�g
		Sodiac_FROM_Get = FlashSodiac;
		return(unsigned char)1;// �擾����
	}
	return(unsigned char)0;// �擾���s
}

//[]----------------------------------------------------------------------[]
///	@brief			SPI FROM�A�N�Z�X���J���i�Z�}�t�H�J���j
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		SPI FROM�̃A�N�Z�X�����J������
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void From_Access_SemFree(unsigned char FlashSodiac)
{	//	0/1/2 = none/Flash/Sodiac
	if (Sodiac_FROM_Get == FlashSodiac) {
		f_from_access_sem = 0; // �A�N�Z�X�t���O�N���A
		Sodiac_FROM_Get = 0;
	}
}
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
///	@brief			RSTEN�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			NMI��������FROM�����Z�b�g���邽�߂̃R�}���h(RSTEN,RST�Z�b�g�Ŏ��s����)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void SetRsten(void)
{
	unsigned char command = 0x66;			// RSTEN
	// �R�}���h���M�t�F�[�Y
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2703;				// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
	RSPI0.SPDR = command;
	while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
	command = (unsigned char) RSPI0.SPDR;	// �_�~�[���[�h
}
//[]----------------------------------------------------------------------[]
///	@brief			RST�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
///	@note			NMI��������FROM�����Z�b�g���邽�߂̃R�}���h(RSTEN,RST�Z�b�g�Ŏ��s����)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void SetRst(void)
{
	unsigned char command = 0x99;			// RST
    // �R�}���h���M�t�F�[�Y
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2703;				// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
	RSPI0.SPDR = command;
	while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
	command = (unsigned char) RSPI0.SPDR;	// �_�~�[���[�h
}

//[]----------------------------------------------------------------------[]
///	@brief			WREN(���C�g�C�l�[�u��)�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		�ʏ퓮��Ŏg�p(���荞�݂ɂ�鑗��M:�^�X�N�؂芷�����삪�n�܂��Ă���g�p�������)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void WriteEnable(void)
{
	Rspi_SndBuf[0] = 0x06;					// WREN
	Rspi_from_SndReq(1);					// ���M1byte�ŗv��
}
//[]----------------------------------------------------------------------[]
///	@brief			RDSR(�X�e�[�^�X���[�h)�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			stat	: �V���A��FROM�̃X�e�[�^�X
///	@author			A.iiizumi
///	@attention		�ʏ퓮��Ŏg�p(���荞�݂ɂ�鑗��M:�^�X�N�؂芷�����삪�n�܂��Ă���g�p�������)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadStatus(void)
{
	unsigned char stat;
	stat = 0xFF;

	Rspi_SndBuf[0] = 0x05;					// RDSR
	Rspi_SndBuf[1] = 0xFF;					// 1Byte��M����̂ŃN���b�N�𑗐M���邽�߃_�~�[�f�[�^�Z�b�g
	Rspi_from_SndReq(2);					// ���M1byte,��M1byte�ŗv��

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	stat = 	Rspi_RcvBuf[1];					// ��M�f�[�^��2Byte�ڂ��X�e�[�^�X
	return(stat);
}

//[]----------------------------------------------------------------------[]
///	@brief			WRSR(�X�e�[�^�X���C�g)�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		val		: �V���A��FROM�̃X�e�[�^�X
///	@return			void
///	@author			A.iiizumi
///	@attention		�ʏ퓮��Ŏg�p(���荞�݂ɂ�鑗��M:�^�X�N�؂芷�����삪�n�܂��Ă���g�p�������)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteStatus(unsigned char val)
{
	WriteEnable();							// WREN(���C�g�C�l�[�u��)
	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	Rspi_SndBuf[0] = 0x01;					// WRSR
	Rspi_SndBuf[1] = val;					// �ݒ�f�[�^
	Rspi_from_SndReq(2);					// ���M2byte�ŗv��

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	/* check status to make sure previous operation completed */
	while( FlashReadStatus()&SR_BSY );
}
//[]----------------------------------------------------------------------[]
///	@brief			�C���[�X�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: �C���[�X�J�n�A�h���X(�I�[���C���[�X�̏ꍇ�͔C��)
///	@param[in]		size	: �C���[�X���e 0:Chip,1:4KB(sector),2:64KB(block)
///	@return			0		: �ُ�I��
///					1		: ����I��
///	@author			A.iiizumi
///	@attention		�ʏ퓮��Ŏg�p(���荞�݂ɂ�鑗��M:�^�X�N�؂芷�����삪�n�܂��Ă���g�p�������)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashErase(const unsigned long address, const unsigned char size)
{
	const unsigned char	erase_commands[] = { 0xc7, 0x20, 0xd8 };	// Chip, 4KB, 64KB

	if(size > 2){
		return (unsigned char)0;			// �v���ԈႢ�̏ꍇ�������Ȃ�
	}

	WriteEnable();							// WREN(���C�g�C�l�[�u��)
	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	Rspi_SndBuf[0] = erase_commands[size];	// �R�}���h�Z�b�g
	if( size>0 ) {	// followed address
		Rspi_SndBuf[1] = (unsigned char)(address >> 16);
		Rspi_SndBuf[2] = (unsigned char)(address >>  8);
		Rspi_SndBuf[3] = (unsigned char)(address >>  0);
		Rspi_from_SndReq(4);				// ���M4byte�ŗv��
	}
	else {
		Rspi_from_SndReq(1);				// ���M1byte�ŗv��
	}

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	/* check status to make sure previous operation completed */
	while( FlashReadStatus()&SR_BSY );

	return (unsigned char)1;				// ����I��
}
//[]----------------------------------------------------------------------[]
///	@brief			���C�g�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: ���C�g�J�n�J�n�A�h���X
///	@param[in]		*data	: �������ރf�[�^�̃|�C���^(�������݃T�C�Y��256Byte�P�ʌŒ�) 
///	@return			void
///	@author			A.iiizumi
///	@attention		�ʏ퓮��Ŏg�p(���荞�݂ɂ�鑗��M:�^�X�N�؂芷�����삪�n�܂��Ă���g�p�������)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteData(unsigned long address, unsigned char const *data)
{
	// �������݃T�C�Y��256Byte�P�ʂ̎d�l
	WriteEnable();							// WREN(���C�g�C�l�[�u��)
	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	memcpy(&Rspi_SndBuf[4],data,256);		// ���M�f�[�^��256Byte���R�s�[
	Rspi_SndBuf[0] = 0x02;					// Program
	Rspi_SndBuf[1] = (unsigned char)(address >> 16);
	Rspi_SndBuf[2] = (unsigned char)(address >>  8);
	Rspi_SndBuf[3] = (unsigned char)(address >>  0);
	Rspi_from_SndReq(260);					// ���M260byte�ŗv��(�R�}���h4,�f�[�^256)

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}
	/* check status to make sure previous operation completed */
	while( FlashReadStatus()&SR_BSY );
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
///	@attention		�ʏ퓮��Ŏg�p(���荞�݂ɂ�鑗��M:�^�X�N�؂芷�����삪�n�܂��Ă���g�p�������)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadData(const unsigned long address, unsigned char *data, const unsigned long size)
{
	unsigned long  data_size;
	unsigned int i ;

	if((size == 0) || (FROM_SECTOR_SIZE < size)) {
		return (unsigned char)0;			// �͈͊O�̏ꍇ�͉������Ȃ�
	}

	memset(Rspi_SndBuf,0xFF,sizeof(Rspi_SndBuf));
	Rspi_SndBuf[0] = 0x03;					// Read
	Rspi_SndBuf[1] = (unsigned char)(address >> 16);
	Rspi_SndBuf[2] = (unsigned char)(address >>  8);
	Rspi_SndBuf[3] = (unsigned char)(address >>  0);
	data_size = size + 4;
	Rspi_from_SndReq(data_size);			// ���M260byte�ŗv��(�R�}���h4,�f�[�^256)

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	for (i = 0; i < size; i++) {
		data[i] = Rspi_RcvBuf[i+4];			// ��M�f�[�^��4Byte�ڂ��炪�f�[�^
	}
	return (unsigned char)1;				// ����I��
}
//[]----------------------------------------------------------------------[]
///	@brief			device identification ���[�h�R�}���h
//[]----------------------------------------------------------------------[]
///	@param[out]		*val	: ���[�h�f�[�^���������ރ|�C���^(3byte)
///	@return			void
///	@author			A.iiizumi
///	@attention		�ʏ퓮��Ŏg�p(���荞�݂ɂ�鑗��M:�^�X�N�؂芷�����삪�n�܂��Ă���g�p�������)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashJEDECReadId(unsigned char *val)
{
	int i ;

	Rspi_SndBuf[0] = 0x9F;					// JEDEC-ID
	Rspi_SndBuf[1] = 0xFF;					// 3Byte��M����̂ŃN���b�N�𑗐M���邽�߃_�~�[�f�[�^�Z�b�g
	Rspi_SndBuf[2] = 0xFF;
	Rspi_SndBuf[3] = 0xFF;
	Rspi_from_SndReq(4);					// ���M1byte,��M3byte�ŗv��

	while(0 == Rspi_from_IsSndRcvCmp()) {
		taskchg( IDLETSKNO );
	}

	for (i = 0; i < 3; i++) {
		val[i] = Rspi_RcvBuf[i+1];			// ��M�f�[�^��2Byte�ڂ��炪�f�[�^
	}

}


//[]----------------------------------------------------------------------[]
///	@brief			WREN(���C�g�C�l�[�u��)�R�}���h(���荞�ݖ��g�p)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		�N������FROM���J�o�������Ŏg�p���邱��(�^�X�N�؂�ւ����n�܂��Ă���͎g�p�֎~)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void WriteEnable_direct(void)
{
	unsigned char command = 0x06;			// WREN

    // �R�}���h���M�t�F�[�Y
    while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2703;				// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
    RSPI0.SPDR = command;
    while (RSPI0.SPSR.BIT.IDLNF) ;
   	command = (unsigned char) RSPI0.SPDR;	// �_�~�[���[�h
}

//[]----------------------------------------------------------------------[]
///	@brief			RDSR(�X�e�[�^�X���[�h)�R�}���h(���荞�ݖ��g�p)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			stat	: �V���A��FROM�̃X�e�[�^�X
///	@author			A.iiizumi
///	@attention		�N������FROM���J�o�������Ŏg�p���邱��(�^�X�N�؂�ւ����n�܂��Ă���͎g�p�֎~)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashReadStatus_direct(void)
{
	unsigned char command = 0x05;			// RDSR
	volatile unsigned char dmmy;

    // �R�}���h���M�t�F�[�Y
    while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// �]���̓r���̎� SSL�M���̓A�N�e�B�u��ێ�
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
    RSPI0.SPDR = command;
	// �f�[�^��M�t�F�[�Y
    while (RSPI0.SPSR.BIT.IDLNF) ;
	RSPI0.SPCMD0.WORD = 0x2703;				// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
	spi_wait();								// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
    RSPI0.SPDR = 0xFF;
    while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
	command = (unsigned char) (RSPI0.SPDR & 0xFF) ;

	return(command);
}
//[]----------------------------------------------------------------------[]
///	@brief			WRSR(�X�e�[�^�X���C�g)�R�}���h(���荞�ݖ��g�p)
//[]----------------------------------------------------------------------[]
///	@param[in]		val		: �V���A��FROM�̃X�e�[�^�X
///	@return			void
///	@author			A.iiizumi
///	@attention		�N������FROM���J�o�������Ŏg�p���邱��(�^�X�N�؂�ւ����n�܂��Ă���͎g�p�֎~)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteStatus_direct(unsigned char val)
{
	unsigned char command[2] = { 0x01, val };// WRSR, value
	volatile unsigned char dmmy;
	WriteEnable_direct();

// �R�}���h���M�t�F�[�Y
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// �]���̓r���̎� SSL�M���̓A�N�e�B�u��ێ�
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
	RSPI0.SPDR = command[0];
	while (RSPI0.SPSR.BIT.IDLNF) ;
	spi_wait();								// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
	RSPI0.SPCMD0.WORD = 0x2703;				// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
	RSPI0.SPDR = command[1];
	while (RSPI0.SPSR.BIT.IDLNF) ;
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h

	/* check status to make sure previous operation completed */
	while( FlashReadStatus_direct()&SR_BSY ) {
		dog_reset();
	}

}
//[]----------------------------------------------------------------------[]
///	@brief			�C���[�X�R�}���h(���荞�ݖ��g�p)
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: �C���[�X�J�n�A�h���X(�I�[���C���[�X�̏ꍇ�͔C��)
///	@param[in]		size	: �C���[�X���e 0:Chip,1:4KB(sector),2:64KB(block)
///	@return			0		: �ُ�I��
///					1		: ����I��
///	@author			A.iiizumi
///	@attention		�N������FROM���J�o�������Ŏg�p���邱��(�^�X�N�؂�ւ����n�܂��Ă���͎g�p�֎~)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned char FlashErase_direct(const unsigned long address, const unsigned char size)
{
	const unsigned char	erase_commands[] =
						{ 0xc7, 0x20, 0xd8 };// Chip, 4KB, 64KB
	unsigned char	n, command[4];
	int i ;
	volatile unsigned char	dmmy;

	if(size > 2) {
		return (unsigned char)0;			// �v���ԈႢ�̏ꍇ�������Ȃ�
	}

	WriteEnable_direct();
	command[0] = erase_commands[size];
	if( size>0 ) {	// followed address
		n = 4;
		command[1] = (unsigned char)(address >> 16);
		command[2] = (unsigned char)(address >>  8);
		command[3] = (unsigned char)(address >>  0);
	}
	else {
		n = 1;
	}
	// �R�}���h���M�t�F�[�Y
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// �]���̓r���̎� SSL�M���̓A�N�e�B�u��ێ�
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
	for (i = 0; i < n; i++) {
		if(i == (n-1)){
			RSPI0.SPCMD0.WORD = 0x2703;		// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
		}
		RSPI0.SPDR = command[i];
		while (RSPI0.SPSR.BIT.IDLNF);
		spi_wait();							// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
		dmmy = (unsigned char) RSPI0.SPDR;	// �_�~�[���[�h
	}

	/* check status to make sure previous operation completed */
	while( FlashReadStatus_direct()&SR_BSY ) {
		dog_reset();
	}
	return (unsigned char)1;				// ����I��
}
//[]----------------------------------------------------------------------[]
///	@brief			���C�g�R�}���h(���荞�ݖ��g�p)
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: ���C�g�J�n�J�n�A�h���X
///	@param[in]		*data	: �������ރf�[�^�̃|�C���^(�������݃T�C�Y��256Byte�P�ʌŒ�) 
///	@return			void
///	@author			A.iiizumi
///	@attention		�N������FROM���J�o�������Ŏg�p���邱��(�^�X�N�؂�ւ����n�܂��Ă���͎g�p�֎~)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashWriteData_direct(unsigned long address, unsigned char const *data)
{
	// �������݃T�C�Y��256Byte�P�ʂ̎d�l
	unsigned char command[5];
	int i ;
	volatile unsigned char	dmmy;
	unsigned long w_size;

	w_size = 256;
	command[0] = 0x02;						// Byte-Program

	WriteEnable_direct();
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

	while( w_size>0 ){
		dog_reset();
		if(w_size == 1){
			RSPI0.SPCMD0.WORD = 0x2703;		// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
		}
		RSPI0.SPDR = *data;
		while (RSPI0.SPSR.BIT.IDLNF);
		spi_wait();							// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
		dmmy = (unsigned char) RSPI0.SPDR;	// �_�~�[���[�h
		/* update address & size values */
		w_size--;
		data++;
	}

	/* check status to make sure previous operation completed */
	while( FlashReadStatus_direct()&SR_BSY ) {
		dog_reset();
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			���[�h�R�}���h(���荞�ݖ��g�p)
//[]----------------------------------------------------------------------[]
///	@param[in]		address	: ���[�h�J�n�J�n�A�h���X
///	@param[out]		*data	: ���[�h�f�[�^���������ރ|�C���^
///	@param[in]		size	: ���[�h�f�[�^�T�C�Y(1�`4096(1�Z�N�^))
///	@return			0		: �ُ�I��
///					1		: ����I��
///	@author			A.iiizumi
///	@attention		�N������FROM���J�o�������Ŏg�p���邱��(�^�X�N�؂�ւ����n�܂��Ă���͎g�p�֎~)
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

	if((size == 0) || (FROM_SECTOR_SIZE < size)) {
		return (unsigned char)0;			// �͈͊O�̏ꍇ�͉������Ȃ�
	}

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
		dog_reset();
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
//[]----------------------------------------------------------------------[]
///	@brief			device identification ���[�h�R�}���h(���荞�ݖ��g�p)
//[]----------------------------------------------------------------------[]
///	@param[out]		*val	: ���[�h�f�[�^���������ރ|�C���^(3byte)
///	@return			void
///	@author			A.iiizumi
///	@attention		�N������FROM���J�o�������Ŏg�p���邱��(�^�X�N�؂�ւ����n�܂��Ă���͎g�p�֎~)
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void FlashJEDECReadId_direct(unsigned char *val)
{
	unsigned char	command = 0x9F;			// JEDEC-ID
	volatile unsigned char dmmy;
	int i ;

	// �R�}���h���M�t�F�[�Y
	while (RSPI0.SPSR.BIT.IDLNF);			// ensure transmit register is empty
	RSPI0.SPCMD0.WORD = 0x2783;				// �]���̓r���̎� SSL�M���̓A�N�e�B�u��ێ�
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
	RSPI0.SPDR = command;
	while (RSPI0.SPSR.BIT.IDLNF);
	spi_wait();								// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
	dmmy = (unsigned char) RSPI0.SPDR;		// �_�~�[���[�h
	// �f�[�^��M�t�F�[�Y
	for (i = 0; i < 3; i++) {
		if(i == 2) {
			RSPI0.SPCMD0.WORD = 0x2703;		// �]���̍Ō�̎� SSL�M���͓]�������ŉ���
		}
		RSPI0.SPDR = 0xFF;
		/* Wait for transfer to complete */
		while (RSPI0.SPSR.BIT.IDLNF) ;		// ensure transmit register is empty
		spi_wait();							// IDLNF���N���A����Ă���wait�������Ȃ���SPDR�̒l���ǂ߂Ȃ�����
		val[i] = (unsigned char) (RSPI0.SPDR & 0xFF) ;
	}
}

