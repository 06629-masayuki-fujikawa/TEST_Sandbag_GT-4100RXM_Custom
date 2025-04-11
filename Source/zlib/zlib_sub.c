#include "system.h"
#include "mem_def.h"
#include "string.h"
#include "zlib.h"
#include "zlib_sub.h"


#define ZLIB_WINDOWBITS			12			//
#define ZLIB_MEM_LEVEL			6			// 1�`9 1=�ŏ��̃������g�p 9=�ő�̃������g�p
											// ���̒l�𑝂₷�ꍇ�́Amem_block�̗e�ʂ�ύX����K�v������
#define FLATE_ONETIME_SIZE		(10*1024)	// 10K

// GG120600(S) // Phase9 �𓀎���WindowBit���f�t�H���g��
// struct _ST_COMP_MEM {
// 	char mem_used[5];
// 	char block1[7200]; // 7200
// 	char block2[8202]; // 4096 * 2 + 10
// 	char block3[8202]; // 4096 * 2 + 10
// 	char block4[16394]; // 8192 * 2 + 10
// 	char block5[16394]; // 4096 * 4 + 10
// } mem_block;
union{
	struct _ST_COMP_MEM {
		char mem_used[5];
		char block1[7200]; // 7200
		char block2[8202]; // 4096 * 2 + 10
		char block3[8202]; // 4096 * 2 + 10
		char block4[16394]; // 8192 * 2 + 10
		char block5[16394]; // 4096 * 4 + 10
	} mem_block1;
	struct _ST_COMP_MEM2 {
		char mem_used[5];
		char block1[7200]; // 7200
		char block2[8192+4096+10]; // 4096 * 3 + 10
		char block3[4096+10]; // 4096 * 1 + 10
		char block4[8192*4 +10 ]; // 8192 * 4 + 10
		char block5[10]; // 4096 * 4 + 10
	} mem_block2;
}mem;

// ��������������
/// zconf.h
/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus about 7 kilobytes
 for small objects.
*/

#define mem_block		mem.mem_block1
#define mem_block2		mem.mem_block2

// GG120600(E) // Phase9 �𓀎���WindowBit���f�t�H���g��

static int DoCompUnComp(int nType, unsigned char *indata,unsigned char *outdata, int inSize,int outSize);


voidpf (DoAllocFunc) OF((voidpf opaque, uInt items, uInt size))
{
	voidpf retpf = NULL;
	uInt allsize = items * size;

	if (mem_block.mem_used[0] == 0 && allsize <= sizeof(mem_block.block1))
	{
		retpf = mem_block.block1;
		mem_block.mem_used[0] = 1;
	}
	else if (mem_block.mem_used[1] == 0 && allsize <= sizeof(mem_block.block2))
	{
		retpf = mem_block.block2;
		mem_block.mem_used[1] = 2;
	}
	else if (mem_block.mem_used[2] == 0 && allsize <= sizeof(mem_block.block3))
	{
		retpf = mem_block.block3;
		mem_block.mem_used[2] = 3;
	}
	else if (mem_block.mem_used[3] == 0 && allsize <= sizeof(mem_block.block4))
	{
		retpf = mem_block.block4;
		mem_block.mem_used[3] = 1;
	}
	else if (mem_block.mem_used[4] == 0 && allsize <= sizeof(mem_block.block5))
	{
		retpf = mem_block.block5;
		mem_block.mem_used[4] = 1;
	}

	return retpf;
}

void (DoFreeFunc) OF((voidpf opaque, voidpf address))
{
	if (address == mem_block.block1)
	{
		mem_block.mem_used[0] = 0;
	}
	else if (address == mem_block.block2)
	{
		mem_block.mem_used[1] = 0;
	}
	else if (address == mem_block.block3)
	{
		mem_block.mem_used[2] = 0;
	}
	else if (address == mem_block.block4)
	{
		mem_block.mem_used[3] = 0;
	}
	else if (address == mem_block.block5)
	{
		mem_block.mem_used[4] = 0;
	}
}
// GG120600(S) // Phase9 �𓀎���WindowBit���f�t�H���g��
voidpf (DoAllocFunc2) OF((voidpf opaque, uInt items, uInt size))
{
	voidpf retpf = NULL;
	uInt allsize = items * size;

	if (mem_block2.mem_used[0] == 0 && allsize <= sizeof(mem_block2.block1))
	{
		retpf = mem_block2.block1;
		mem_block2.mem_used[0] = 1;
	}
	else if (mem_block2.mem_used[1] == 0 && allsize <= sizeof(mem_block2.block2))
	{
		retpf = mem_block2.block2;
		mem_block2.mem_used[1] = 2;
	}
	else if (mem_block2.mem_used[2] == 0 && allsize <= sizeof(mem_block2.block3))
	{
		retpf = mem_block2.block3;
		mem_block2.mem_used[2] = 3;
	}
	else if (mem_block2.mem_used[3] == 0 && allsize <= sizeof(mem_block2.block4))
	{
		retpf = mem_block2.block4;
		mem_block2.mem_used[3] = 1;
	}
	else if (mem_block2.mem_used[4] == 0 && allsize <= sizeof(mem_block2.block5))
	{
		retpf = mem_block2.block5;
		mem_block2.mem_used[4] = 1;
	}

	return retpf;
}

void (DoFreeFunc2) OF((voidpf opaque, voidpf address))
{
	if (address == mem_block2.block1)
	{
		mem_block2.mem_used[0] = 0;
	}
	else if (address == mem_block2.block2)
	{
		mem_block2.mem_used[1] = 0;
	}
	else if (address == mem_block2.block3)
	{
		mem_block2.mem_used[2] = 0;
	}
	else if (address == mem_block2.block4)
	{
		mem_block2.mem_used[3] = 0;
	}
	else if (address == mem_block2.block5)
	{
		mem_block2.mem_used[4] = 0;
	}
}
// GG120600(E) // Phase9 �𓀎���WindowBit���f�t�H���g��

int zlib_Compress(unsigned char *indata,unsigned char *outdata, int inSize,int outSize)
{
	// ���k
	return DoCompUnComp(FLATE_TYPE_DEFLATE,indata,outdata,inSize,outSize);
}

int zlib_Uncompress(unsigned char *indata,unsigned char *outdata, int inSize,int outSize)
{
	// ��
	return DoCompUnComp(FLATE_TYPE_INFLATE,indata,outdata,inSize,outSize);
}


static int DoCompUnComp(int nType, unsigned char *indata,unsigned char *outdata, int inSize,int outSize)
{
	z_stream zstrm;
	int ret_id = 0;
	int flush = Z_NO_FLUSH;
	int inTotalSize = 0;
	int nTempSize = inSize;
	int (*_DoFlate)(z_stream*,int );		// �֐��|�C���^

	if (inSize > outSize || inSize <= 0 || outSize <= 0)
	{
		return -1;
	}
	if( inSize > FLATE_ONETIME_SIZE){
		nTempSize = FLATE_ONETIME_SIZE;
	}
	memset( outdata, 0, outSize);
	memset( &zstrm, 0, sizeof(z_stream));

// GG120600(S) // Phase9 �𓀎���WindowBit���f�t�H���g��
//	zstrm.zalloc = DoAllocFunc;
//	zstrm.zfree = DoFreeFunc;
// GG120600(E) // Phase9 �𓀎���WindowBit���f�t�H���g��
	zstrm.opaque = Z_NULL;
	// ������
	if(nType == FLATE_TYPE_DEFLATE){
// GG120600(S) // Phase9 �𓀎���WindowBit���f�t�H���g��
		zstrm.zalloc = DoAllocFunc;
		zstrm.zfree = DoFreeFunc;
// GG120600(E) // Phase9 �𓀎���WindowBit���f�t�H���g��
		// ���k
		deflateInit2_(&zstrm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, ZLIB_WINDOWBITS, ZLIB_MEM_LEVEL ,
			Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(zstrm));
		_DoFlate = deflate;
	}else{
		// ��
// GG120600(S) // Phase9 �𓀎���WindowBit���f�t�H���g��
//		inflateInit2_(&zstrm, ZLIB_WINDOWBITS, ZLIB_VERSION, (int)sizeof(zstrm));
		zstrm.zalloc = DoAllocFunc2;
		zstrm.zfree = DoFreeFunc2;

		inflateInit_(&zstrm, ZLIB_VERSION, (int)sizeof(zstrm));
// GG120600(E) // Phase9 �𓀎���WindowBit���f�t�H���g��
		_DoFlate = inflate;
	}

	zstrm.next_in = indata;
	zstrm.avail_in = nTempSize;
	zstrm.next_out = outdata;
	zstrm.avail_out = outSize;

	while ( ( ret_id = _DoFlate( &zstrm, flush ) ) != Z_STREAM_END )
	{
		 switch ( ret_id )
		 {
		 case	Z_OK:			 // ���Ȃ�
		 case	Z_BUF_ERROR:	 // ����or�o�̓o�b�t�@����
			// ���̓o�b�t�@�̗��p�ł���T�C�Y��0
			if ( zstrm.avail_in == 0 )
			{
				inTotalSize += nTempSize;
				if( inTotalSize >= inSize){
					// ���̓t�@�C���̏I�[�ɒB�����瓮���ύX����
					flush = Z_FINISH;
				}
				if( (inTotalSize + nTempSize) >= inSize){
					nTempSize = inSize - inTotalSize;
					// ���̓t�@�C���̏I�[�ɒB�����瓮���ύX����
					flush = Z_FINISH;
				}
				// ���p�ł���T�C�Y�X�V
				zstrm.avail_in = nTempSize;
			}
			// �o�̓o�b�t�@�̗��p�ł���T�C�Y��0
			if ( zstrm.avail_out == 0)
			{	// �G���[
				return -1;
			}
			break;
		case	Z_NEED_DICT:	// ����
		case	Z_DATA_ERROR:	// �s���f�[�^
		case	Z_STREAM_ERROR:	// �G���[
		default:
			return -1;
		}
		WACDOG;
// GG120600(S) // Phase9 ZIP����taskchg�ǉ�
		taskchg(IDLETSKNO);
// GG120600(E) // Phase9 ZIP����taskchg�ǉ�
	}
	if(nType == FLATE_TYPE_DEFLATE){
		deflateEnd(&zstrm);
	}else{
		inflateEnd(&zstrm);
	}

	return (int) zstrm.total_out;

}

