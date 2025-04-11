#ifndef	_SUB_TYPE_
#define	_SUB_TYPE_
//[]----------------------------------------------------------------------[]
///	@mainpage		
///	environment structures<br><br>
///	<b>Copyright(C) 2007 AMANO Corp.</b>
///	CREATE			2007/07/04 MATSUSHITA<br>
///	UPDATE			
///	@file			subtype.h
///	@date			2007/07/04
///	@version		MH644410
//[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]

// �@�\ID
enum {
		FID_BASE = 0x11
	,	FID_rNTNET = 0x31
	,	FID_NTNET = 0xD0
	,	FID_SNTP = 0xE0
	,	FID_TELNET
	,	FID_FTP
	,	FID_FTPtran
};

// ���s�敪
enum {
	_RUN_AUTO,
	_RUN_MANUAL
};

#pragma	pack
typedef	struct {
			unsigned char	type[3];	// ���ʎq
			unsigned long	proglen;	// �v���O�����T�C�Y
			unsigned short	sum;		// �`�F�b�N�T��
			unsigned char	version[9];	// �v���O�����o�[�W����
} PRG_HDR;
#pragma unpack

// ���f�[�^
typedef	struct {
		PRG_HDR		hdr;
	// ���s�@�\�ԍ��P�`�P�Q�̋@�\
		struct {
			unsigned char	ID;			// �@�\ID
			unsigned char	run;		// ���s�敪
		} func[12];
	// �ȉ������g��
} DEF_ENV;

// �o�[�W�������
#pragma	pack
typedef	struct {
		unsigned long	size;			// = sizeof(DEF_VER)
		char			version[10];	// �v���O�����o�[�W����
		char			dummy[2];		// 16�̔{���ɂȂ�悤��
// �ȉ��C�ӊg��
} DEF_VER;
#pragma unpack

// �@�\���
typedef	struct {
		unsigned short	ID;				// �@�\ID
		unsigned short	reserved;
		void	(*start)(int fno);		// �v���O�����J�n�֐�
} DEF_FNC;


/* SUB CPU Mapping */
#define	ENV_TOP		0x0c000000		/* environment data top addr. */
#define	PRG_TOP		0x0c010000		/* program area top addr. */

/* MAIN CPU Mapping(flash offset) */
#define	VECT_SIZE	(256*4)						/* vector size */
#define	PRG_OFS		(sizeof(PRG_HDR))			/* program offset */
#define	VER_OFS		(PRG_OFS+VECT_SIZE)			/* version info. offset */

#endif
