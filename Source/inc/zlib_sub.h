#ifndef ZLIB_SUB_H
#define ZLIB_SUB_H

#define FLATE_TYPE_DEFLATE		0			// ���k
#define FLATE_TYPE_INFLATE		1			// ��

int zlib_Compress(unsigned char *indata,unsigned char *outdata, int inSize,int outSize);
int zlib_Uncompress(unsigned char *indata,unsigned char *outdata, int inSize,int outSize);


#endif
