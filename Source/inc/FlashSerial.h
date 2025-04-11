#ifndef _FLASHSERIAL_
#define _FLASHSERIAL_
//[]----------------------------------------------------------------------[]
///	@file		FlashSerial.h
///	@brief		�V���A���t���b�V��ROM SPI�֘A��`�w�b�_
///	@date		2012/02/09
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

//******************************************************************************
// Typedef definitions
//******************************************************************************
/** ��M�o�b�t�@�Ǘ��e�[�u�� **/
typedef struct {
    ushort  RcvReqCnt;					// ��M�v���o�C�g��
    ushort  RcvCmpCnt;					// ��M�����o�C�g��
    ushort  RcvCmpFlg;					// ��M���������t���O 1=����,0=������
} t_RspiRcvCtrl;

/** ���M�o�b�t�@�Ǘ��e�[�u�� **/
typedef struct {
    ushort  SndReqCnt;					// ���M�v���o�C�g��
    ushort  SndCmpCnt;					// ���M�����o�C�g��
    ushort  ReadIndex;					// next send data (read) index
    ushort  SndCmpFlg;					// ���M���������t���O 1=����,0=������
} t_RspiSndCtrl;

//******************************************************************************
// Macro definitions
//******************************************************************************

/* Status Register Bit Definitions */
#define SR_BSY			0x01
#define SR_BPL			0x08
#define SR_BP_SET(x)	(x<<2)	// BPL=0,BP3-0=x

#define FROM_SECTOR_SIZE	4096
// SPI FROM �o�b�t�@�T�C�Y(���[�h�R�}���h03H�A�R�}���h1Byte�A�A�h���X3Byte�A�f�[�^4096(4K)byte���ő�Ƃ��邽��)
#define RSPI_BUF_SIZE	(FROM_SECTOR_SIZE + 4) 

//******************************************************************************
// extern definitions
//******************************************************************************

// memdeta.c
extern	t_RspiRcvCtrl	Rspi_RcvCtrl;				// RSPI��M�Ǘ����(���荞�݃n���h���g�p)
extern	t_RspiSndCtrl	Rspi_SndCtrl;				// RSPI���M�Ǘ����(���荞�݃n���h���g�p)
extern	unsigned char	Rspi_RcvBuf[RSPI_BUF_SIZE];// RSPI��M�o�b�t�@(���荞�݃n���h���g�p)
extern	unsigned char	Rspi_SndBuf[RSPI_BUF_SIZE];// RSPI���M�o�b�t�@(���荞�݃n���h���g�p)

// FlashSerialSST.c
extern unsigned char From_Access_SemGet(unsigned char FlashSodiac);
extern void From_Access_SemFree(unsigned char FlashSodiac);
extern void SetRsten(void);
extern void SetRst(void);
extern unsigned char FlashReadStatus(void);
extern void FlashWriteStatus( unsigned char );
extern unsigned char  FlashErase ( const unsigned long, const unsigned char );
extern unsigned char  FlashReadData ( const unsigned long, unsigned char *, const unsigned long );
extern void FlashWriteData ( unsigned long, unsigned char const *);
extern void FlashJEDECReadId ( unsigned char * );

extern unsigned char FlashReadStatus_direct(void);
extern void FlashWriteStatus_direct(unsigned char );
extern unsigned char FlashErase_direct(const unsigned long , const unsigned char );
extern void FlashWriteData_direct(unsigned long , unsigned char const *);
extern unsigned char FlashReadData_direct(const unsigned long , unsigned char *, const unsigned long );
extern void FlashJEDECReadId_direct(unsigned char *);

// FlashSerial_RSPI.c
extern void Rspi_from_Init(void);
extern void Rspi_from_SPRI( void );
extern unsigned char Rspi_from_SndReq( unsigned short Length );
extern unsigned char Rspi_from_IsSndRcvCmp( void );
#endif
