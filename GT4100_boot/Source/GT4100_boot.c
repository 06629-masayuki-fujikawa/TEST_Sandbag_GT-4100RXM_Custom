//[]----------------------------------------------------------------------[]
///	@file		FT4000_boot.c
///	@brief		�u�[�g�v���O�������C������
/// @date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
/*--- Include -------------------*/
#include <machine.h>
#include <string.h>
#include "iodefine.h"
#include "s_dataflash.h"
#include "s_FlashSerial.h"
#include "s_memdata.h"
/*--- Pragma --------------------*/


/*--- Define --------------------*/
#define MAIN_PROGRAM_ADDR				0xFFE00000	// (����)���C���v���O�����擪�A�h���X
#define MAIN_PROGRAM_ADDR_WR_ENTRY3		0x00E00000L	// (����)�������ݎ��̃��C���v���O�����̈�3�擪�A�h���X
#define MAIN_PROGRAM_ADDR_WR_ENTRY2		0x00E80000L	// (����)�������ݎ��̃��C���v���O�����̈�2�擪�A�h���X
#define MAIN_PROGRAM_ADDR_WR_ENTRY1		0x00F00000L	// (����)�������ݎ��̃��C���v���O�����̈�1�擪�A�h���X
#define MAIN_PROGRAM_ADDR_WR_ENTRY0		0x00F80000L	// (����)�������ݎ��̃��C���v���O�����̈�0�擪�A�h���X


#define BOOT_PROGRAM_BLOCK_COUNT		3									// �u�[�g�v���O�����̃Z�N�^��EB0�`EB3
#define MAIN_PROGRAM_ELASE_BLOCK_NUM	(70 - BOOT_PROGRAM_BLOCK_COUNT)		// (����)���C���v���O�����̃C���[�X����u���b�N�̐� EB3�`EB69
																			// EB0�`EB2��3�u���b�N�̓u�[�g�v���O�������g�̂��߃C���[�X���Ȃ�
#define MAIN_PROGRAM_ELASE_BLOCK_ENTRY3 8 									// (����)FENTRY3�̃u���b�N�̐� EB62�`EB69
#define MAIN_PROGRAM_ELASE_BLOCK_ENTRY2 8 									// (����)FENTRY2�̃u���b�N�̐� EB54�`EB61
#define MAIN_PROGRAM_ELASE_BLOCK_ENTRY1 16 									// (����)FENTRY1�̃u���b�N�̐� EB38�`EB53
#define MAIN_PROGRAM_ELASE_BLOCK_ENTRY0 (38 - BOOT_PROGRAM_BLOCK_COUNT)		// (����)FENTRY0�̃u���b�N�̐� EB03�`EB37


#define MAIN_PROGRAM_WR_UNIT 128				// (����)���C���v���O�����ɑ΂��ď������ޒP��  128byte


#define MAIN_PROGRAM_EXFROM_ADDR	0x00E00000L	// �_�E�����[�h�������C���v���O�����̊O��FROM�i�[�擪�A�h���X
#define MAIN_PROGRAM_EXFROM_SECTOR_CNT	512		// �_�E�����[�h�������C���v���O�����̊O��FROM�̃Z�N�^��(4K 512�Z�N�^��2Mbyte)
// �_�E�����[�h�������C���v���O����������������Z�N�^��(�u�[�g�̈�͏���)
#define MAIN_PROGRAM_UPDATE_SECTOR_CNT (MAIN_PROGRAM_EXFROM_SECTOR_CNT - BOOT_PROGRAM_BLOCK_COUNT)

#define MAIN_PROGRAM_EXFROM_SECTOR_SIZE	4096	// �_�E�����[�h�������C���v���O�����̊O��FROM�̃Z�N�^�T�C�Y(4K 4096byte)


/*** Standby time data *******************************************************/
// �n�[�h�E�F�A�}�j���A���̓d�C�I�������MAX�̑҂����Ԃ��Z�o�������A���ۂɂ͏��������悤�Ȃ̂�
// �}�[�W����4�{�Ƃ���B(1.1�{�̃}�[�W���Ƃ������C���[�X��WAIT_TE16K�Ń^�C���A�E�g�������������߁A�^�C���A�E�g�������͖̂��Ȃ�)
#define WAIT_TE16K ((unsigned long)(288000 * 4 * (50.0/PCLK_FREQUENCY)) * ICLK_FREQUENCY / 24)
	//tE16K ��MAX���Ԃ��u288ms�v�Ƃ����ꍇ��WAIT�J�E���g�i���24�̓v���O������CLK�����l���j

#define WAIT_TP128 ((unsigned long)(12000 * 4 * (50.0/PCLK_FREQUENCY)) * ICLK_FREQUENCY / 24)
	//tP128 ��MAX���Ԃ��u12ms�v�Ƃ����ꍇ��WAIT�J�E���g�i���24�̓v���O������CLK�����l���j
	
#define WAIT_TRESW2 ((unsigned long)(35 * 4 * (50.0/PCLK_FREQUENCY)) * ICLK_FREQUENCY / 12)
	//tRESW2 ��MAX���Ԃ��u35us�v�Ƃ����ꍇ��WAIT�J�E���g�i���12�̓v���O������CLK�����l���j

#define WAIT_TPCKA ((unsigned long)(60 * 4 * (50.0/PCLK_FREQUENCY)) * ICLK_FREQUENCY / 24)
	//tPCKA ��MAX���Ԃ��u60us�v�Ƃ����ꍇ��WAIT�J�E���g�i���24�̓v���O������CLK�����l���j

/*** FCU firmware & FCU-RAM information **************************************/
#define FCU_FIRM_TOP			0xFEFFE000		// FCU firmware store area start address = FEFFE000h
#define FCU_RAM_TOP				0x007F8000		// FCU RAM start address = 007F8000h
#define FCU_RAM_SIZE			0x2000			// Size of FCU RAM = 8 Kbytes

/*** Index of FCU MODE *******************************************************/
enum{
	FCU_ROM_READ_MODE = 0,
	FCU_ROM_PE_MODE,
};

/*--- Gloval Value --------------*/

/*****************************************************************************/
/*  Declaration of internal data type                                        */
/*****************************************************************************/
typedef enum {																// FCU state information
	FCU_SUCCESS,															// - 0 : FCU success state
	FCU_ERROR,																// - 1 : FCU error state
} FCU_STATUS;

typedef struct {															// FCU information
	unsigned short *p_write_buffer;											// - Address of wrdata_buffer[]
	volatile __evenaccess unsigned char *p_command_adrs;					// - Address to issue FCU commands
	unsigned short *p_write_adrs_top;										// - Start address for programming/erasure
	unsigned short *p_write_adrs_end;										// - End address for programming/erasure
	unsigned short *p_write_adrs_now;										// - Programming address for the programming process
} ST_FCU_INFO;

typedef struct st_eb_adrs {													// Erasure block information
	unsigned long eb_write_adrs_top;										//  - Start address for programming/erasure of target EB
} ST_EB_ADRS;

typedef union {
	unsigned short write_buf[2048];
	unsigned char  read_buf[4096];
} BF;

BF	from_data;
ST_FCU_INFO fcu_info;														// FCU infomation store area (36 bytes)
unsigned char FcuMode = FCU_ROM_READ_MODE;
unsigned char update_state;



/*--- Prototype -----------------*/
/*--- ROM��Ŏ��s����֐� ---*/
void main(void);															// Main program
/*--- RAM��Ŏ��s����֐� ---*/
void err_led( unsigned char kind);
void dog_init( void );
void Flash_Update(void);													// User mat programming/erasure control program
void fcu_Interrupt_Disable(void);											// FCU interrupt requests disabled control program
void fcu_Reset(void);														// FCU initialization control program
FCU_STATUS fcu_Transfer_Firmware(ST_FCU_INFO *p_fcu_info);					// Transfer FCU firmware to FCU-RAM control program
FCU_STATUS fcu_Transition_RomRead_Mode(ST_FCU_INFO *p_fcu_info);			// Transition to ROM read mode control program
FCU_STATUS fcu_Transition_RomPE_Mode(unsigned char entry);					// Transition to ROM P/E mode control program
FCU_STATUS fcu_Notify_Peripheral_Clock(ST_FCU_INFO *p_fcu_info);			// Notification peripheral module clock(PCLK) to FCU control program
FCU_STATUS fcu_Erase(ST_FCU_INFO *p_fcu_info);								// Erasure control program
FCU_STATUS fcu_Write(ST_FCU_INFO *p_fcu_info);								// Programming control program


/*--- Extern --------------------*/

/*****************************************************************************/
/*  Constant value definition                                                */
/*****************************************************************************/
/*** Erasure block information data ******************************************/
ST_EB_ADRS tbl_eb_adrs[] = {		// Erasure block information
									// - EB#  : Start address R (W)   to End address R (W)     size
//�̈�3(FENTRY3)
	{0x00E00000},					// - EB69 : FFE00000h (00E00000h) to FFE0FFFFh (00E0FFFFh) 64 Kbytes
	{0x00E10000},					// - EB68 : FFE10000h (00E10000h) to FFE1FFFFh (00E1FFFFh) 64 Kbytes
	{0x00E20000},					// - EB67 : FFE20000h (00E20000h) to FFE2FFFFh (00E2FFFFh) 64 Kbytes
	{0x00E30000},					// - EB66 : FFE30000h (00E30000h) to FFE3FFFFh (00E3FFFFh) 64 Kbytes
	{0x00E40000},					// - EB65 : FFE40000h (00E40000h) to FFE4FFFFh (00E4FFFFh) 64 Kbytes
	{0x00E50000},					// - EB64 : FFE50000h (00E50000h) to FFE5FFFFh (00E5FFFFh) 64 Kbytes
	{0x00E60000},					// - EB63 : FFE60000h (00E60000h) to FFE6FFFFh (00E6FFFFh) 64 Kbytes
	{0x00E70000},					// - EB62 : FFE70000h (00E70000h) to FFE7FFFFh (00E7FFFFh) 64 Kbytes
//�̈�2(FENTRY2)
	{0x00E80000},					// - EB61 : FFE80000h (00E80000h) to FFE8FFFFh (00E8FFFFh) 64 Kbytes
	{0x00E90000},					// - EB60 : FFE90000h (00E90000h) to FFE9FFFFh (00E9FFFFh) 64 Kbytes
	{0x00EA0000},					// - EB59 : FFEA0000h (00EA0000h) to FFEAFFFFh (00EAFFFFh) 64 Kbytes
	{0x00EB0000},					// - EB58 : FFEB0000h (00EB0000h) to FFEBFFFFh (00EBFFFFh) 64 Kbytes
	{0x00EC0000},					// - EB57 : FFEC0000h (00EC0000h) to FFECFFFFh (00ECFFFFh) 64 Kbytes
	{0x00ED0000},					// - EB56 : FFED0000h (00ED0000h) to FFEDFFFFh (00EDFFFFh) 64 Kbytes
	{0x00EE0000},					// - EB55 : FFEE0000h (00EE0000h) to FFEEFFFFh (00EEFFFFh) 64 Kbytes
	{0x00EF0000},					// - EB54 : FFEF0000h (00EF0000h) to FFEFFFFFh (00EFFFFFh) 64 Kbytes
//�̈�1(FENTRY1)
	{0x00F00000},					// - EB53 : FFF00000h (00F00000h) to FFF07FFFh (00F07FFFh) 32 Kbytes
	{0x00F08000},					// - EB52 : FFF08000h (00F08000h) to FFF0FFFFh (00F0FFFFh) 32 Kbytes
	{0x00F10000},					// - EB51 : FFF10000h (00F10000h) to FFF17FFFh (00F17FFFh) 32 Kbytes
	{0x00F18000},					// - EB50 : FFF18000h (00F18000h) to FFF1FFFFh (00F1FFFFh) 32 Kbytes
	{0x00F20000},					// - EB49 : FFF20000h (00F20000h) to FFF27FFFh (00F27FFFh) 32 Kbytes
	{0x00F28000},					// - EB48 : FFF28000h (00F28000h) to FFF2FFFFh (00F2FFFFh) 32 Kbytes
	{0x00F30000},					// - EB47 : FFF30000h (00F30000h) to FFF37FFFh (00F37FFFh) 32 Kbytes
	{0x00F38000},					// - EB46 : FFF88000h (00F38000h) to FFF3FFFFh (00F3FFFFh) 32 Kbytes
	{0x00F40000},					// - EB45 : FFF40000h (00F40000h) to FFF47FFFh (00F47FFFh) 32 Kbytes
	{0x00F48000},					// - EB44 : FFF48000h (00F48000h) to FFF4FFFFh (00F4FFFFh) 32 Kbytes
	{0x00F50000},					// - EB43 : FFF50000h (00F50000h) to FFF57FFFh (00F57FFFh) 32 Kbytes
	{0x00F58000},					// - EB42 : FFF58000h (00F58000h) to FFF5FFFFh (00F5FFFFh) 32 Kbytes
	{0x00F60000},					// - EB41 : FFF60000h (00F60000h) to FFF67FFFh (00F67FFFh) 32 Kbytes
	{0x00F68000},					// - EB40 : FFF68000h (00F68000h) to FFF6FFFFh (00F6FFFFh) 32 Kbytes
	{0x00F70000},					// - EB39 : FFF70000h (00F70000h) to FFF77FFFh (00F77FFFh) 32 Kbytes
	{0x00F78000},					// - EB38 : FFF78000h (00F78000h) to FFF7FFFFh (00F7FFFFh) 32 Kbytes
//�̈�0(FENTRY0)
	{0x00F80000},					// - EB37 : FFF80000h (00F80000h) to FFF83FFFh (00F83FFFh) 16 Kbytes
	{0x00F84000},					// - EB36 : FFF84000h (00F84000h) to FFF87FFFh (00F87FFFh) 16 Kbytes
	{0x00F88000},					// - EB35 : FFF88000h (00F88000h) to FFF8BFFFh (00F8BFFFh) 16 Kbytes
	{0x00F8C000},					// - EB34 : FFF8C000h (00F8C000h) to FFF8FFFFh (00F8FFFFh) 16 Kbytes
	{0x00F90000},					// - EB33 : FFF90000h (00F90000h) to FFF93FFFh (00F93FFFh) 16 Kbytes
	{0x00F94000},					// - EB32 : FFF94000h (00F94000h) to FFF97FFFh (00F97FFFh) 16 Kbytes
	{0x00F98000},					// - EB31 : FFF98000h (00F98000h) to FFF9BFFFh (00F9BFFFh) 16 Kbytes
	{0x00F9C000},					// - EB30 : FFF9C000h (00F9C000h) to FFF9FFFFh (00F9FFFFh) 16 Kbytes
	{0x00FA0000},					// - EB29 : FFFA0000h (00FA0000h) to FFFA3FFFh (00FA3FFFh) 16 Kbytes
	{0x00FA4000},					// - EB28 : FFFA4000h (00FA4000h) to FFFA7FFFh (00FA7FFFh) 16 Kbytes
	{0x00FA8000},					// - EB27 : FFFA8000h (00FA8000h) to FFFABFFFh (00FABFFFh) 16 Kbytes
	{0x00FAC000},					// - EB26 : FFFAC000h (00FAC000h) to FFFAFFFFh (00FAFFFFh) 16 Kbytes
	{0x00FB0000},					// - EB25 : FFFB0000h (00FB0000h) to FFFB3FFFh (00FB3FFFh) 16 Kbytes
	{0x00FB4000},					// - EB24 : FFFB4000h (00FB4000h) to FFFB7FFFh (00FB7FFFh) 16 Kbytes
	{0x00FB8000},					// - EB23 : FFFB8000h (00FB8000h) to FFFBBFFFh (00FBBFFFh) 16 Kbytes
	{0x00FBC000},					// - EB22 : FFFBC000h (00FBC000h) to FFFBFFFFh (00FBFFFFh) 16 Kbytes
	{0x00FC0000},					// - EB21 : FFFC0000h (00FC0000h) to FFFC3FFFh (00FC3FFFh) 16 Kbytes
	{0x00FC4000},					// - EB20 : FFFC4000h (00FC4000h) to FFFC7FFFh (00FC7FFFh) 16 Kbytes
	{0x00FC8000},					// - EB19 : FFFC8000h (00FC8000h) to FFFCBFFFh (00FCBFFFh) 16 Kbytes
	{0x00FCC000},					// - EB18 : FFFCC000h (00FCC000h) to FFFCFFFFh (00FCFFFFh) 16 Kbytes
	{0x00FD0000},					// - EB17 : FFFD0000h (00FD0000h) to FFFD3FFFh (00FD3FFFh) 16 Kbytes
	{0x00FD4000},					// - EB16 : FFFD4000h (00FD4000h) to FFFD7FFFh (00FD7FFFh) 16 Kbytes
	{0x00FD8000},					// - EB15 : FFFD8000h (00FD8000h) to FFFDBFFFh (00FDBFFFh) 16 Kbytes
	{0x00FDC000},					// - EB14 : FFFDC000h (00FDC000h) to FFFDFFFFh (00FDFFFFh) 16 Kbytes
	{0x00FE0000},					// - EB13 : FFFE0000h (00FE0000h) to FFFE3FFFh (00FE3FFFh) 16 Kbytes
	{0x00FE4000},					// - EB12 : FFFE4000h (00FE4000h) to FFFE7FFFh (00FE7FFFh) 16 Kbytes
	{0x00FE8000},					// - EB11 : FFFE8000h (00FE8000h) to FFFEBFFFh (00FEBFFFh) 16 Kbytes
	{0x00FEC000},					// - EB10 : FFFEC000h (00FEC000h) to FFFEFFFFh (00FEFFFFh) 16 Kbytes
	{0x00FF0000},					// - EB09 : FFFF0000h (00FF0000h) to FFFF3FFFh (00FF3FFFh) 16 Kbytes
	{0x00FF4000},					// - EB08 : FFFF4000h (00FF4000h) to FFFF7FFFh (00FF7FFFh) 16 Kbytes
	{0x00FF8000},					// - EB07 : FFFF8000h (00FF8000h) to FFFF8FFFh (00FF8FFFh)  4 Kbytes
	{0x00FF9000},					// - EB06 : FFFF9000h (00FF9000h) to FFFF9FFFh (00FF9FFFh)  4 Kbytes
	{0x00FFA000},					// - EB05 : FFFFA000h (00FFA000h) to FFFFAFFFh (00FFAFFFh)  4 Kbytes
	{0x00FFB000},					// - EB04 : FFFFB000h (00FFB000h) to FFFFBFFFh (00FFBFFFh)  4 Kbytes
	{0x00FFC000},					// - EB03 : FFFFC000h (00FFC000h) to FFFFCFFFh (00FFCFFFh)  4 Kbytes
	{0x00FFD000},					// - EB02 : FFFFD000h (00FFD000h) to FFFFDFFFh (00FFDFFFh)  4 Kbytes
	{0x00FFE000},					// - EB01 : FFFFE000h (00FFE000h) to FFFFEFFFh (00FFEFFFh)  4 Kbytes
	{0x00FFF000}					// - EB00 : FFFFF000h (00FFF000h) to FFFFFFFFh (00FFFFFFh)  4 Kbytes
};

//[]----------------------------------------------------------------------[]
///	@brief			�u�[�g�v���O�������C������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/03/29<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void main(void)
{
	unsigned int work;														//�A�h���X�w��p

	// ���[�U�[�v���O�����̃A�h���X��ݒ肷��
	unsigned short (*UserProgram)(void);
	work = MAIN_PROGRAM_ADDR;
	UserProgram = (unsigned short (*)(void))work;
	EXIO_CS4_EXIO = 0x1041;// �|�[�g�o�͏����l�Z�b�g
	CP_OUTREG_EN = 0;// �O���o�͗L��
	// �v���O�����X�V�t���O����
	if(0 == memcmp(BootInfo.f_prog_update,prog_update_flag,sizeof(BootInfo.f_prog_update))) {// �v���O�����X�V�t���O�L
		// �v���O�����̏�������
		/* ���[�U�}�b�g������/��������v���O���������RAM�ɃR�s�[ */
		memcpy(__sectop("RF_UPDATE_FUNC"), __sectop("PF_UPDATE_FUNC"), ((__secsize("PF_UPDATE_FUNC")+3)/4)*4);
		/* ����RAM��̃��[�U�}�b�g������/����v���O�����ɃW�����v */
		Flash_Update();

		BootInfo.err_count = 0;// �G���[�J�E���^���N���A
		memset(BootInfo.f_prog_update,0x00,sizeof(BootInfo.f_prog_update));// �X�V�t���O�N���A
		UserProgram();//���[�U�[�v���O�����ł���0xfffe0000�փW�����v
	} else {
		// �A�b�v�f�[�g�����ł��邪�A�g�p���Ă���t���O��J�E���^�̓N���A���Ă���
		BootInfo.err_count = 0;// �G���[�J�E���^���N���A
		memset(BootInfo.f_prog_update,0x00,sizeof(BootInfo.f_prog_update));// �X�V�t���O�N���A
		UserProgram();//���[�U�[�v���O�����ł���0xfffe0000�փW�����v
	}

}

#pragma section F_UPDATE_FUNC// �v���O�����������ݎ��v���O���������RAM�ɃR�s�[����̈�

//[]----------------------------------------------------------------------[]
///	@brief			�G���[LED�_������
//[]----------------------------------------------------------------------[]
///	@param[in]		0:�������ɃG���[��~�A1:�����t�ŃG���[��~
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void err_led( unsigned char kind )
{
	if(kind == 0){
		EXIO_CS4_EXIO = 0x1049;// ST3�_��
		CP_OUTREG_EN = 0;// �O���o�͗L��
		BootInfo.err_count = 0;// �G���[�J�E���^���N���A
		// �X�V�t���O���N���A���邱�Ƃɂ��ċN�����ɍX�V�������s�Ȃ�Ȃ��悤�ɂ���
		memset(BootInfo.f_prog_update,0x00,sizeof(BootInfo.f_prog_update));// �X�V�t���O�N���A
	}else{
		// �����t�̏ꍇ���Z�b�g���s���g�[�^��10��G���[�����������ꍇ�ACPU�̓���FROM�����Ă���Ɣ��f���G���[��~�Ƃ���
		BootInfo.err_count++;
		if(BootInfo.err_count >= 10){
			// �����s�\�Ɣ��f��������~
			EXIO_CS4_EXIO = 0x1049;// ST3�_��
			CP_OUTREG_EN = 0;// �O���o�͗L��
			BootInfo.err_count = 0;// �G���[�J�E���^���N���A
			// �X�V�t���O���N���A���邱�Ƃɂ��ċN�����ɍX�V�������s�Ȃ�Ȃ��悤�ɂ���
			memset(BootInfo.f_prog_update,0x00,sizeof(BootInfo.f_prog_update));// �X�V�t���O�N���A
		}else{
			// FCU�֘A�̋����I�ȃG���[�����������ꍇ���l������UCPU�����Z�b�g���A
			// �u�[�g�����̃V�[�P���X�����߂�����Ȃ������Ƃŕ��������݂�
			dog_init();
		}
	}
	for(;;);
}
//[]----------------------------------------------------------------------[]
///	@brief			WDG�N������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/05/28<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void dog_init( void )
{
	// Timeout���ԑI�� :1024Cycle
	// �����I��: Clock50MHz/4
	// 1024*4/50,000,000 = 81.92(us)	Timeout����
	// Window�I���ʒu�F�w��Ȃ�
	// Window�J�n�ʒu�F�w��Ȃ�
	WDT.WDTCR.WORD = 0x3310;// WDTCR�͋N����1�񂵂��������߂Ȃ��d�l�Ȃ���WORD�P�ʂŏ�������

	WDT.WDTRCR.BIT.RSTIRQS	= 1;	// ���Z�b�g�o��
	WDT.WDTRR	= 0x00;				//
	WDT.WDTRR	= 0xFF;				// refresh
}
//[]----------------------------------------------------------------------[]
///	@brief			�v���O����������������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Flash_Update(void)
{

	FCU_STATUS fcu_status;													// FCU status data storage area (enum)
	unsigned short buf_cnt;													// wrdata_buffer counter
	unsigned long RomAddress_inwr;											// ����FROM�������ݗp�A�h���X
	unsigned long RomAddress_inrd;											// ����FROM�ǂݏo���p�A�h���X
	unsigned short cnt;														// wrdata_buffer counter
	unsigned long read_RomAddr;												// �O��FROM���[�h�p�擪�A�h���X
	unsigned char f_rom_pe;													// PE���[�h�Ɉڍs����Ƃ�FENTRY0�`FENTRY3�̂ǂ��ɃA�N�Z�X���邩�w�肷��t���O
	//--- FCU information initialization ---

	// fcu_info�p�����[�^�ݒ� (�����ݒ�p)
	fcu_info.p_command_adrs   = (unsigned char *)tbl_eb_adrs[0].eb_write_adrs_top;
	fcu_info.p_write_adrs_top = (unsigned short *)tbl_eb_adrs[0].eb_write_adrs_top;
	fcu_info.p_write_adrs_now = (unsigned short *)tbl_eb_adrs[0].eb_write_adrs_top;

	fcu_Interrupt_Disable();												// FCU���荞�݂��֎~��Ԃɐݒ�

	fcu_status = fcu_Transfer_Firmware(&fcu_info);							// ��FCU�t�@�[���E�F�A��FCU RAM�̈�ɃR�s�[����
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}

	fcu_status = fcu_Transition_RomPE_Mode(3);								// ��FCU��ROM P/E���[�h�֑J�ڂ�����(FENTRY3���w��)
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_PE_MODE;												// FCU���[�h�ϐ���ROM P/E���[�h��

	fcu_status = fcu_Notify_Peripheral_Clock(&fcu_info);					// ��PCLK�̎��g�������W�X�^�ɐݒ肵�A���ӃN���b�N�ʒm�R�}���h�𔭍s����
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}

	// �����v���O�����̃C���[�X����
	// �̈�3�̃C���[�X
	for(cnt = 0 ; cnt < MAIN_PROGRAM_ELASE_BLOCK_ENTRY3 ; cnt++) {
		fcu_info.p_command_adrs   = (unsigned char *)tbl_eb_adrs[cnt].eb_write_adrs_top;
		fcu_status = fcu_Erase(&fcu_info);									// �u���b�N�C���[�Y�̎��s
		if (fcu_status == FCU_ERROR) {										// FCU�X�e�[�^�X�G���[����
			err_led(1);														// �G���[LED�_��
		}
	}
	fcu_status = fcu_Transition_RomRead_Mode(&fcu_info);					// ��FCU��ROM read���[�h�֑J�ڂ�����
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_READ_MODE;											// FCU���[�h�ϐ���ROM���[�h���[�h��

	// �̈�2�̃C���[�X
	fcu_status = fcu_Transition_RomPE_Mode(2);								// ��FCU��ROM P/E���[�h�֑J�ڂ�����(FENTRY2���w��)
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_PE_MODE;												// FCU���[�h�ϐ���ROM P/E���[�h��
	for(; cnt < MAIN_PROGRAM_ELASE_BLOCK_ENTRY3+MAIN_PROGRAM_ELASE_BLOCK_ENTRY2 ; cnt++) {
		fcu_info.p_command_adrs   = (unsigned char *)tbl_eb_adrs[cnt].eb_write_adrs_top;
		fcu_status = fcu_Erase(&fcu_info);									// �u���b�N�C���[�Y�̎��s
		if (fcu_status == FCU_ERROR) {										// FCU�X�e�[�^�X�G���[����
			err_led(1);														// �G���[LED�_��
		}
	}
	fcu_status = fcu_Transition_RomRead_Mode(&fcu_info);					// ��FCU��ROM read���[�h�֑J�ڂ�����
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_READ_MODE;											// FCU���[�h�ϐ���ROM���[�h���[�h��
	
	// �̈�1�̃C���[�X
	fcu_status = fcu_Transition_RomPE_Mode(1);								// ��FCU��ROM P/E���[�h�֑J�ڂ�����(FENTRY1���w��)
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_PE_MODE;												// FCU���[�h�ϐ���ROM P/E���[�h��
	for(; cnt < MAIN_PROGRAM_ELASE_BLOCK_ENTRY3+MAIN_PROGRAM_ELASE_BLOCK_ENTRY2+MAIN_PROGRAM_ELASE_BLOCK_ENTRY1 ; cnt++) {
		fcu_info.p_command_adrs   = (unsigned char *)tbl_eb_adrs[cnt].eb_write_adrs_top;
		fcu_status = fcu_Erase(&fcu_info);									// �u���b�N�C���[�Y�̎��s
		if (fcu_status == FCU_ERROR) {										// FCU�X�e�[�^�X�G���[����
			err_led(1);														// �G���[LED�_��
		}
	}
	fcu_status = fcu_Transition_RomRead_Mode(&fcu_info);					// ��FCU��ROM read���[�h�֑J�ڂ�����
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_READ_MODE;											// FCU���[�h�ϐ���ROM���[�h���[�h��

	// �̈�0�̃C���[�X
	fcu_status = fcu_Transition_RomPE_Mode(0);								// ��FCU��ROM P/E���[�h�֑J�ڂ�����(FENTRY0���w��)
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_PE_MODE;												// FCU���[�h�ϐ���ROM P/E���[�h��
	for(; cnt < MAIN_PROGRAM_ELASE_BLOCK_ENTRY3+MAIN_PROGRAM_ELASE_BLOCK_ENTRY2+MAIN_PROGRAM_ELASE_BLOCK_ENTRY1+MAIN_PROGRAM_ELASE_BLOCK_ENTRY0 ; cnt++) {
		fcu_info.p_command_adrs   = (unsigned char *)tbl_eb_adrs[cnt].eb_write_adrs_top;
		fcu_status = fcu_Erase(&fcu_info);									// �u���b�N�C���[�Y�̎��s
		if (fcu_status == FCU_ERROR) {										// FCU�X�e�[�^�X�G���[����
			err_led(1);														// �G���[LED�_��
		}
	}
	fcu_status = fcu_Transition_RomRead_Mode(&fcu_info);					// ��FCU��ROM read���[�h�֑J�ڂ�����
	if (fcu_status == FCU_ERROR) {											// FCU�X�e�[�^�X�G���[����
		err_led(1);															// �G���[LED�_��
	}
	FcuMode = FCU_ROM_READ_MODE;											// FCU���[�h�ϐ���ROM���[�h���[�h��

	RomAddress_inwr = MAIN_PROGRAM_ADDR_WR_ENTRY3;							// �������ݎ��̐擪�A�h���X���Z�b�g
	RomAddress_inrd = MAIN_PROGRAM_ADDR;									// �ǂݏo�����̐擪�A�h���X���Z�b�g
	for(cnt = 0, read_RomAddr = MAIN_PROGRAM_EXFROM_ADDR ; cnt < MAIN_PROGRAM_UPDATE_SECTOR_CNT ; cnt++,read_RomAddr += MAIN_PROGRAM_EXFROM_SECTOR_SIZE) {
		FlashReadData_direct(read_RomAddr, &from_data.read_buf[0], MAIN_PROGRAM_EXFROM_SECTOR_SIZE);// �O��FROM����1�Z�N�^���f�[�^�����[�h

		// �ŏ���FENTRY0�`FENTRY3�̂ǂ��ɑ΂��ăA�N�Z�X���Ă��邩�𔻒f����B
		if(RomAddress_inwr < MAIN_PROGRAM_ADDR_WR_ENTRY2) {
			// ���݂̃A�N�Z�X�͗̈�3
			f_rom_pe = 3;
			fcu_info.p_command_adrs   = (unsigned char *)MAIN_PROGRAM_ADDR_WR_ENTRY3;
		} else if(RomAddress_inwr < MAIN_PROGRAM_ADDR_WR_ENTRY1) {
			// ���݂̃A�N�Z�X�͗̈�2
			f_rom_pe = 2;
			fcu_info.p_command_adrs   = (unsigned char *)MAIN_PROGRAM_ADDR_WR_ENTRY2;
		} else if(RomAddress_inwr < MAIN_PROGRAM_ADDR_WR_ENTRY0) {
			// ���݂̃A�N�Z�X�͗̈�1
			f_rom_pe = 1;
			fcu_info.p_command_adrs   = (unsigned char *)MAIN_PROGRAM_ADDR_WR_ENTRY1;
		} else {
			// ���݂̃A�N�Z�X�͗̈�0
			f_rom_pe = 0;
			fcu_info.p_command_adrs   = (unsigned char *)MAIN_PROGRAM_ADDR_WR_ENTRY0;
		}

		fcu_status = fcu_Transition_RomPE_Mode(f_rom_pe);				// ��FCU��ROM P/E���[�h�֑J�ڂ�����
		if (fcu_status == FCU_ERROR) {									// FCU�X�e�[�^�X�G���[����
			err_led(1);													// �G���[LED�_��
		}
		FcuMode = FCU_ROM_PE_MODE;										// FCU���[�h�ϐ���ROM P/E���[�h��

		// �����v���O�����̏������ݏ���
		for( buf_cnt = 0 ;buf_cnt < (MAIN_PROGRAM_EXFROM_SECTOR_SIZE/2) ; buf_cnt += (MAIN_PROGRAM_WR_UNIT/2), RomAddress_inwr += MAIN_PROGRAM_WR_UNIT ) {
			fcu_info.p_write_adrs_now = (unsigned short *)RomAddress_inwr;
			fcu_info.p_write_buffer   = &from_data.write_buf[buf_cnt];
			fcu_status = fcu_Write(&fcu_info);								// ���Ώۂ�EB��256byte�����݂��s��
			if (fcu_status == FCU_ERROR) {									// FCU�X�e�[�^�X�G���[����
				err_led(1);													// �G���[LED�_��
			}
		}

		// �����v���O�����̃x���t�@�C�`�F�b�N����
		fcu_status = fcu_Transition_RomRead_Mode(&fcu_info);				// ��FCU��ROM read���[�h�֑J�ڂ�����
		if (fcu_status == FCU_ERROR) {										// FCU�X�e�[�^�X�G���[����
			err_led(1);														// �G���[LED�_��
		}
		FcuMode = FCU_ROM_READ_MODE;										// FCU���[�h�ϐ���ROM���[�h���[�h��

		for (buf_cnt = 0; buf_cnt < MAIN_PROGRAM_EXFROM_SECTOR_SIZE; buf_cnt ++) {
			if(from_data.read_buf[buf_cnt] != *((unsigned char *)RomAddress_inrd)) {
				break;														// ��M�f�[�^��ROM�f�[�^�s��v��
			}
			RomAddress_inrd++;
		}

		if(buf_cnt != MAIN_PROGRAM_EXFROM_SECTOR_SIZE) {					// �x���t�@�C�`�F�b�N�G���[
			// �x���t�@�C�`�F�b�N�͐��폑���ɂ�������炸�l���s��v�Ȃ̂�
			// CPU����FROM���ُ�Ɣ��f���A���g���C�͂����ɋ����I���Ƃ���
			err_led(0);														// �G���[LED�_��
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			FCU���荞�݋֎~����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void fcu_Interrupt_Disable(void)
{
	FLASH.FRDYIE.BIT.FRDYIE = 0;															// Enable or disable the flash ready interrupt(FRDYI) output
																							// Set "0" to FRDYIE.FRDYIE (value after reset = 0)
																							// - FRDYIE interrupt requests "disabled"
	// �t���b�V���A�N�Z�X�G���[���荞�݋����W�X�^
	FLASH.FAEINT.BIT.ROMAEIE = 0;															// Enable or disable the FIFERR interrupt requests when a ROM access violation occurs and
																							// the ROMAE bit in FASTAT is set to 1.
																							// Set "0" to FAEINT.ROMAEIE (value after reset = 1)
																							// - FIFERR interrupt requests "disabled" when the ROMAE bit in FASTAT is set to 1.

	FLASH.FAEINT.BIT.CMDLKIE = 0;															// Enable or disable the FIFERR interrupt requests when a FCU command lock occurs and the
																							// CMDLK bit in FASTAT is set to 1.
																							// Set "0" to FAEINT.CMDLKIE (value after reset = 1)
																							// - FIFERR interrupt requests "disabled" when the CMDLK bit in FASTAT is set to 1.

	FLASH.FAEINT.BIT.DFLAEIE = 0;															// Enable or disable the FIFERR interrupt requests when a data flash access violation occurs
																							// and the DFLAE bit in FASTAT is set to 1.
																							// Set "0" to FAEINT.DFLAEIE (value after reset = 1)
																							// - FIFERR interrupt requests "disabled" when the DFLAE bit in FASTAT is set to 1.

	FLASH.FAEINT.BIT.DFLRPEIE = 0;															// Enable or disable the FIFERR interrupt requests when a data flash read protection violation
																							// occurs and the DFLRPE bit in FASTAT is set to 1.
																							// Set "0" to FAEINT.DFLRPEIE (value after reset = 1)
																							// - FIFERR interrupt requests "disabled" when the DFLRPE bit in FASTAT is set to 1.

	FLASH.FAEINT.BIT.DFLWPEIE = 0;															// Enable or disable the FIFERR interrupt requests when a data flash programming/erasure
																							// protection violation occurs and the DFLWPE bit in FASTAT is set to 1.
																							// Set "0" to FAEINT.DFLWPEIE (value after reset = 1)
																							// - FIFERR interrupt requests "disabled" when the DFLWPE bit in FASTAT is set to 1.

	IPR(FCU,FIFERR) = 0;																	// Set the priority level of the FIFERR interrupt source
																							// Set "0000b" to IPR01.IPR[3:0] (value after reset = 0000b)
																							// - FIFERR interrupt priority level is "level 0 (interrupt prohibited)"

	IEN(FCU,FIFERR) = 0;																	// Enable or disable the FIFERR interrupt source
																							// Set "0" to IER02.IEN5 (value after reset = 0)
																							// - FIFERR interrupt request is "disabled".

	IPR(FCU,FRDYI) = 0;																		// Set the priority level of the FRDYI interrupt source
																							// Set "0000b" to IPR02.IPR[3:0] (value after reset = 0000b)
																							// - FRDYI interrupt priority level is "level 0 (interrupt prohibited)"

	IEN(FCU,FRDYI) = 0;																		// Enable or disable the FRDYI interrupt source
																							// Set "0" to IER02.BIT.IEN7 (value after reset = 0)
																							// - FRDYI interrupt request is "disabled".
}

//[]----------------------------------------------------------------------[]
///	@brief			FCU���Z�b�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			A.iiizumi
///	@attention		
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void fcu_Reset(void)
{
	unsigned long wait_cnt;																	// Loop counter for while-loop

	FLASH.FRESETR.WORD = 0xCC01;															// Initialize the FCU Set "CC01h" to FRESETR (word access)- FCU is reset
	wait_cnt = WAIT_TRESW2;																	// wait_cnt = standby time data for tRESW2 timeout control
	while (wait_cnt != 0) {																	// tRESW2 timeout control
		wait_cnt --;
	}
	FLASH.FRESETR.WORD = 0xCC00;															// Set "CC00h" to FRESETR (word access)- FCU is not reset.
}

//[]----------------------------------------------------------------------[]
///	@brief			FCU�t�@�[���E�F�A��FCU RAM�̈�ɃR�s�[���鏈��
//[]----------------------------------------------------------------------[]
///	@param[in]		ST_FCU_INFO : FCU�Ǘ��̈�̃|�C���^
///	@return			FCU_ERROR   : �ُ�I��
///					FCU_SUCCESS	: ����I��
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
FCU_STATUS fcu_Transfer_Firmware(ST_FCU_INFO *p_fcu_info)
{
	FCU_STATUS ret;																			// FCU status information
	ret = FCU_SUCCESS;

	if (FLASH.FENTRYR.WORD != 0x0000) {														// If the reading value of the FENTRYR is not "0000h",
		ret = fcu_Transition_RomRead_Mode(p_fcu_info);										// Transition to ROM read mode(call of fcu_Transition_RomRead_Mode function).
		if (ret == FCU_ERROR) {																// If the fcu_status is "FCU_ERROR", do return.
			return ret;
		}
	}

	FLASH.FCURAME.WORD = 0xC401;															//Write "C401h" to FCURAME (word access)- Access to the FCU RAM "enabled". 
	memcpy((void *)FCU_RAM_TOP, (void *)FCU_FIRM_TOP, (unsigned long)FCU_RAM_SIZE);
																							// Transfer the FCU firmware to FCU RAM area
																							// - FCU_RAM_TOP  = 007F 8000h (Transfer destination)
																							// - FCU_FIRM_TOP = FEFF E000h (Transfer source)
																							// - FCU_RAM_SIZE = 2000h      (Size of FCU RAM area)
																							// - Transfer by using the memcpy fucnction.
																							// - "memcpy function" -- (comlile) --> "SMOVF instruction"
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			FCU��ROM���[�h���[�h�ɂ��鏈��
//[]----------------------------------------------------------------------[]
///	@param[in]		ST_FCU_INFO : FCU�Ǘ��̈�̃|�C���^
///	@return			FCU_ERROR   : �ُ�I��
///					FCU_SUCCESS	: ����I��
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
FCU_STATUS fcu_Transition_RomRead_Mode(ST_FCU_INFO *p_fcu_info)
{
	FCU_STATUS ret;																			// FCU status information
	unsigned long wait_cnt;																	// Loop counter for while-loop
	ret = FCU_SUCCESS;

	wait_cnt = WAIT_TE16K;																	// wait_cnt = standby time data for tE16K x1.1 timeout control
	while (FLASH.FSTATR0.BIT.FRDY == 0) {													// If the reading value of the FSTATR0.FRDY is not "1", do the tE16K x1.1 timeout control
		wait_cnt --;
		if (wait_cnt == 0) {																// If the wait_cnt value is "0", do the FCU initialization processing
			fcu_Reset();																	// Initialize the FCU (call of fcu_Reset function)
			FLASH.FENTRYR.WORD = 0xAA00;													// Write "AA00h" to FENTRYR (word access) - Transition to ROM read mode
			FLASH.FWEPROR.BYTE = 0x02;														// Write "02h" to FWEPROR - Protect the execution of the flash write/erase with software
			ret = FCU_ERROR;																// Do the error processing
			return ret;
		}
	}

	if ((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1) || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
		// When one of the ILGLERR, ERSERR and PRGERR bits in FSATAR0 is "1", issue a status register clear command
		if (FLASH.FSTATR0.BIT.ILGLERR == 1) {												// If ILGLERR bit in FSTATR0 is "1", also check the values of the ROMAE, DFLAE, DFLRPE, and DFLWPE bits in FASTAT
			if (FLASH.FASTAT.BYTE != 0x10) {												// If FASTAT is not "10h", write "10h" to FASTAT.
				FLASH.FASTAT.BYTE = 0x10;
			}
		}
		*p_fcu_info->p_command_adrs = 0x50;													// Issue a status register clear command. (Write "50h" to ROM programing/erasure address in byte acces)
	}
	FLASH.FENTRYR.WORD = 0xAA00;															// Write "AA00h" to FENTRYR (word access) - Transition to ROM read mode
	FLASH.FWEPROR.BYTE = 0x02;																// Write "02h" to FWEPROR - Protect the execution of the flash write/erase with software
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			FCU��ROM P/E���[�h�ɂ��鏈��
//[]----------------------------------------------------------------------[]
///	@param[in]		FENTRYR�̃G���g���[bit 0:FENTRY0 1:FENTRY1 2:FENTRY2 3:FENTRY3
///	@return			FCU_ERROR   : �ُ�I��
///					FCU_SUCCESS	: ����I��
///	@author			A.iiizumi
///	@attention		
///	@note			����FROM�̃v���O�����̈��FENTRY0�`FENTRY3�Ŏw�肳���4�̃u���b�N�ō\��
///					����邪�A�S�Ẵu���b�N�ɑ΂��ē����ɃA�N�Z�X���邱�Ƃ͂ł��Ȃ��d�g�݂�
///					�Ȃ��Ă��邽��FENTRY0�`FENTRY3�̃r�b�g������ύX���Ȃ��琧�䂷��
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
FCU_STATUS fcu_Transition_RomPE_Mode(unsigned char entry)
{
	FCU_STATUS ret;																			// FCU status information

	ret = FCU_SUCCESS;

	FLASH.FENTRYR.WORD = 0xAA00;															// Write "AA00h" to FENTRYR (word access)

	switch(entry){
		case 0:
			FLASH.FENTRYR.WORD = 0xAA01;													// Write "AA01h" to FENTRYR (word access)
			break;
		case 1:
			FLASH.FENTRYR.WORD = 0xAA02;													// Write "AA02h" to FENTRYR (word access)
			break;
		case 2:
			FLASH.FENTRYR.WORD = 0xAA04;													// Write "AA04h" to FENTRYR (word access)
			break;
		case 3:
			FLASH.FENTRYR.WORD = 0xAA08;													// Write "AA08h" to FENTRYR (word access)
			break;
		default:
			FLASH.FENTRYR.WORD = 0xAA01;													// Write "AA01h" to FENTRYR (word access)
			break;
	}

	FLASH.FWEPROR.BYTE = 0x01;																// Write "01h" to FWEPROR
																							// - Write/erase "enabled"

	if ((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1) || (FLASH.FSTATR0.BIT.PRGERR == 1)) {
		// When one of the ILGLERR, ERSERR and PRGERR bits in FSTATR0 is 1, do the error processing
		ret = FCU_ERROR;
	}

	if (FLASH.FSTATR1.BIT.FCUERR == 1) {													// If FCUERR bit in FSTATR1 is 1, do the error processing
		ret = FCU_ERROR;
	}

    return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief	PCLK�̎��g�������W�X�^�ɐݒ肵�A���ӃN���b�N�ʒm�R�}���h�𔭍s���鏈��
//[]----------------------------------------------------------------------[]
///	@param[in]		ST_FCU_INFO : FCU�Ǘ��̈�̃|�C���^
///	@return			FCU_ERROR   : �ُ�I��
///					FCU_SUCCESS	: ����I��
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
FCU_STATUS fcu_Notify_Peripheral_Clock(ST_FCU_INFO *p_fcu_info)
{
	FCU_STATUS ret;																			// FCU status information
	unsigned long wait_cnt;																	// Loop counter for while-loop
	ret = FCU_SUCCESS;
	FLASH.PCKAR.WORD = PCLK_FREQUENCY;														// Notify the sequencer of the frequency setting data of the peripheral clock(PCLK) at the
																							//	 programming/erasure for the ROM/data flash.
																							//	 Write "PCLK_FREQUENCY" to PCKAR
																							//	 - PCLK frequency = 48MHz
																							// Issue a peripheral clock notification command
	*p_fcu_info->p_command_adrs = 0xE9;														// - 1st cycle : Write "E9h" to ROM programming/erasure address in byte access
	*p_fcu_info->p_command_adrs = 0x03;														// - 2nd cycle : Write "03h" to ROM programming/erasure address in byte access
	*(volatile __evenaccess unsigned short *)p_fcu_info->p_command_adrs = 0x0F0F;			// - 3rd cycle : Write "0F0Fh" to ROM programming/erasure address in word access
	*(volatile __evenaccess unsigned short *)p_fcu_info->p_command_adrs = 0x0F0F;			// - 4th cycle : Write "0F0Fh" to ROM programming/erasure address in word access
	*(volatile __evenaccess unsigned short *)p_fcu_info->p_command_adrs = 0x0F0F;			// - 5th cycle : Write "0F0Fh" to ROM programming/erasure address in word access
	*p_fcu_info->p_command_adrs = 0xD0;														// - 6th cycle : Write "D0h" to ROM programming/erasure address in byte access

	wait_cnt = WAIT_TPCKA;																	// wait_cnt = standby time data for tPCKA timeout control
	while (FLASH.FSTATR0.BIT.FRDY == 0) {													// If the reading value of the FSTATR0.FRDY is not "1", do the tPCKA timeout control
		wait_cnt --;
		if (wait_cnt == 0) {																// If the wait_cnt value is "0", do the FCU initialization processing
			fcu_Reset();																	// Initialize the FCU (call of fcu_Reset function)
			ret = FCU_ERROR;																// Do the error processing
			return ret;
		}
	}
	if (FLASH.FSTATR0.BIT.ILGLERR == 1) {													// If ILGLERR bit in FSTATR0 is "1",  do the error processing
		ret = FCU_ERROR;
	}
    return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief	�����v���O�����C���[�X����
//[]----------------------------------------------------------------------[]
///	@param[in]		ST_FCU_INFO : FCU�Ǘ��̈�̃|�C���^
///	@return			FCU_ERROR   : �ُ�I��
///					FCU_SUCCESS	: ����I��
///	@author			A.iiizumi
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
FCU_STATUS fcu_Erase(ST_FCU_INFO *p_fcu_info)
{
	FCU_STATUS ret;																			// FCU status information
	unsigned long wait_cnt;																	// Loop counter for while-loop

	ret = FCU_SUCCESS;

	FLASH.FWEPROR.BYTE = 0x01;																// Write "01h" to FWEPROR - Write/erase "enabled"
	FLASH.FPROTR.WORD = 0x5501;																// Write "5501h" to FPROTR (word access)- Protection with a lock bit "disabled"
																							// Issue a block erasure command
	*p_fcu_info->p_command_adrs = 0x20;														// - 1st cycle : Write "20h" to ROM programming/erasure address in byte access
	*p_fcu_info->p_command_adrs = 0xD0;														// - 2nd cycle : Write "D0h" to arbitrary address in erasure block in byte access

	wait_cnt = WAIT_TE16K;																	// wait_cnt = standby time data for tE16k x1.1 timeout control
	while (FLASH.FSTATR0.BIT.FRDY == 0) {													// If the reading value of the FSTATR0.FRDY is not "1", do the tE16k x1.1 timeout control
		wait_cnt --;
		if (wait_cnt == 0) {																// If the wait_cnt value is "0", do the FCU initialization processing
			fcu_Reset();																	// Initialize the FCU (call of fcu_Reset function)
			ret = FCU_ERROR;																// Do the error processing
			return ret;
		}
	}
	if ((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR == 1)) {
		ret = FCU_ERROR;																	// If ILGLERR or ERSERR bit in FSTATR0 is "1", do the error processing
	}

    return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief	�����v���O�����������ݏ���
//[]----------------------------------------------------------------------[]
///	@param[in]		ST_FCU_INFO : FCU�Ǘ��̈�̃|�C���^
///	@return			FCU_ERROR   : �ُ�I��
///					FCU_SUCCESS	: ����I��
///	@author			A.iiizumi
///	@attention		
///	@note			128Byte��WORD�P�ʂŏ�������
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/04/05<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
FCU_STATUS fcu_Write(ST_FCU_INFO *p_fcu_info)
{
	FCU_STATUS ret;																			// FCU status information
	unsigned short *p_write_from;															// Programming source address
	volatile __evenaccess unsigned short *p_write_to;										// Programming destination address
	unsigned long cycle_cnt;																// Loop counter for while-loop
	unsigned long wait_cnt;																	// Loop counter for while-loop

	ret = FCU_SUCCESS;

	p_write_from = (unsigned short *)p_fcu_info->p_write_buffer;							// p_write_from initialization
	p_write_to = (volatile __evenaccess unsigned short *)p_fcu_info->p_write_adrs_now;		// p_write_to initialization
	cycle_cnt = 4;																			// cycle_cnt initialization

	FLASH.FWEPROR.BYTE = 0x01;																// Write "01h" to FWEPROR - Write/erase "enabled"
	FLASH.FPROTR.WORD = 0x5501;																// Write "5501h" to FPROTR (word access) - Protection with a lock bit "disabled"
																							// Issue a program command
	*p_fcu_info->p_command_adrs = 0xE8;														// - 1st cycle   : Write "E8h" to ROM programming/erasure address in byte access 
	*p_fcu_info->p_command_adrs = 0x40;														// - 2nd cycle   : Write "40h" to ROM programming/erasure address in byte access
	*(volatile __evenaccess unsigned short *)p_fcu_info->p_write_adrs_now = *p_write_from;	// - 3rd cycle   : Write the programming data to start address of programming destination in word access 

	while (cycle_cnt <= 66) {																// - 4th to 66h cycle 
		p_write_from ++;																	//               : Write the programming data in 127 word-unit rounds to ROM 
		p_write_to ++;																		//                 programming/erasure address in word access 
		*p_write_to = *p_write_from;
		cycle_cnt ++;
	}

	*p_fcu_info->p_command_adrs = 0xD0;														// - 67th cycle : Write "D0h" to ROM programming/erasure address in byte accesss 

	wait_cnt = WAIT_TP128;																	// wait_cnt = standby time data for tP128 x1.1 timeout control
	while (FLASH.FSTATR0.BIT.FRDY == 0) {													// If the reading value of the FSTATR0.FRDY is not "1", do the tP256 x1.1 timeout control
		wait_cnt --;
		if (wait_cnt == 0) {																// If the wait_cnt value is "0", do the FCU initialization processing 
			fcu_Reset();																	// Initialize the FCU (call of fcu_Reset function)
			ret = FCU_ERROR;																// Do the error processing
			return ret;
		}
	}
	if ((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.PRGERR == 1)) {				// If ILGLERR or PRGERR bit in FSTATR0 is "1", do the error processing
		ret = FCU_ERROR;
	}
    return ret;
}

