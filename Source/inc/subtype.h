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

// 機能ID
enum {
		FID_BASE = 0x11
	,	FID_rNTNET = 0x31
	,	FID_NTNET = 0xD0
	,	FID_SNTP = 0xE0
	,	FID_TELNET
	,	FID_FTP
	,	FID_FTPtran
};

// 実行区分
enum {
	_RUN_AUTO,
	_RUN_MANUAL
};

#pragma	pack
typedef	struct {
			unsigned char	type[3];	// 識別子
			unsigned long	proglen;	// プログラムサイズ
			unsigned short	sum;		// チェックサム
			unsigned char	version[9];	// プログラムバージョン
} PRG_HDR;
#pragma unpack

// 環境データ
typedef	struct {
		PRG_HDR		hdr;
	// 実行機能番号１～１２の機能
		struct {
			unsigned char	ID;			// 機能ID
			unsigned char	run;		// 実行区分
		} func[12];
	// 以下随時拡張
} DEF_ENV;

// バージョン情報
#pragma	pack
typedef	struct {
		unsigned long	size;			// = sizeof(DEF_VER)
		char			version[10];	// プログラムバージョン
		char			dummy[2];		// 16の倍数になるように
// 以下任意拡張
} DEF_VER;
#pragma unpack

// 機能情報
typedef	struct {
		unsigned short	ID;				// 機能ID
		unsigned short	reserved;
		void	(*start)(int fno);		// プログラム開始関数
} DEF_FNC;


/* SUB CPU Mapping */
#define	ENV_TOP		0x0c000000		/* environment data top addr. */
#define	PRG_TOP		0x0c010000		/* program area top addr. */

/* MAIN CPU Mapping(flash offset) */
#define	VECT_SIZE	(256*4)						/* vector size */
#define	PRG_OFS		(sizeof(PRG_HDR))			/* program offset */
#define	VER_OFS		(PRG_OFS+VECT_SIZE)			/* version info. offset */

#endif
