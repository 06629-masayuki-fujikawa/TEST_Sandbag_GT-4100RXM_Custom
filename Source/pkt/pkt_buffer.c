// MH810100(S) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
//[]----------------------------------------------------------------------[]
///	@brief			�p�P�b�g�ʐM�o�b�t�@�Ǘ����W���[��
//[]----------------------------------------------------------------------[]
///	@author			m.onouchi
///	@date			Create	: 2009/09/25
///	@file			pkt_buffer.c
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#include	<string.h>
#include	"system.h"
#include	"common.h"
#include	"message.h"
#include	"ope_def.h"
#include	"pkt_com.h"


#define	PKT_BUFF_SIZ	PKT_CMD_MAX			// �o�b�t�@�T�C�Y���f�[�^���̍ő�T�C�Y

// �\�P�b�g�o�b�t�@�[
typedef	struct {
	ushort	count;
	ushort	total;
	ushort	rp;
	ushort	wp;
	uchar	buff[PKT_BUFF_SIZ];
} PKT_RCVSND_BUFF;

static PKT_RCVSND_BUFF	g_snd_buf;
static PKT_RCVSND_BUFF	g_rcv_buf;
#define snd_buf			g_snd_buf
#define rcv_buf			g_rcv_buf

#define	PKT_RSND_BUFF_SIZ	PKT_CMD_MAX-2	// �f�[�^���̍ő�T�C�Y�|�d����

// �p�P�b�g�đ��M�o�b�t�@�[
typedef struct {
	ushort	count;
	ushort	total;
	ushort	rp;
	ushort	wp;
	uchar	buff[PKT_RSND_BUFF_SIZ];
} PKT_RSND_BUFF;

static PKT_RSND_BUFF	g_rsnd_add_cmd;
#define rsnd_add_cmd	g_rsnd_add_cmd


static void LockPKTSnd( void );
static void UnLockPKTSnd( void );
static void LockPKTRcv( void );
static void UnLockPKTRcv( void );

//[]----------------------------------------------------------------------[]
///	@brief			�o�b�t�@�C�j�V�����C�Y
//[]----------------------------------------------------------------------[]
///	@param			none
///	@return			�Ȃ�
///	@note			�R�}���h���M�o�b�t�@�ƃR�}���h��M�o�b�t�@���N���A���܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PKTbuf_init( void )
{
	memset( &snd_buf, 0, sizeof(snd_buf) );
	memset( &rsnd_add_cmd, 0, sizeof(rsnd_add_cmd) );
	memset( &rcv_buf, 0, sizeof(rcv_buf) );
}

//[]----------------------------------------------------------------------[]
///	@brief			�ŌẪR�}���h���đ��p�o�b�t�@����폜����
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			�R�}���h�T�C�Y(�R�}���h��+�R�}���h�f�[�^��)
///	@attention		�R�}���h�̓R�}���h��(2byte)�ƃR�}���h�f�[�^�ō\������܂��B
///	@note			�o�b�t�@�����O����Ă���R�}���h�̒��ōŌÂ̂��̂��폜����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void PKTbuf_DelReSendCommand( void )
{
	ushort	rp;
	ushort	size, len;
	PKT_RSND_BUFF	*cmd_buf;

	cmd_buf = &rsnd_add_cmd;

	if( cmd_buf->count == 0 ){
		return;
	}

	if( --cmd_buf->count == 0 ){
		// buffer reset
		cmd_buf->total = 0;
		cmd_buf->rp =  0;
		cmd_buf->wp = 0;
		return;
	}

	rp = cmd_buf->rp;
	size = *(ushort*)&cmd_buf->buff[rp];
	len = size;
	rp += (2 + len);
	if( rp >= PKT_RSND_BUFF_SIZ ){
		rp -= PKT_RSND_BUFF_SIZ;
	}
	cmd_buf->rp = rp;
	cmd_buf->total -= (2 + len);
}

//[]----------------------------------------------------------------------[]
///	@brief			���M����R�}���h�f�[�^���đ��p�o�b�t�@�ɃZ�b�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: �Z�b�g����R�}���h�f�[�^
///	@param[in]		len		: �R�}���h��
///	@return			1:OK<br>
///					0:NG
///	@attention		�o�b�t�@���I�[�o�[�t���[����Ƃ��͍ŌẪf�[�^����j������<br>
///					�ŐV�f�[�^��ۑ����܂��B
///	@note			�R�l�N�V�����񕜌�ɍđ�����R�}���h���Ǘ����܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
BOOL PKTbuf_SetReSendCommand( const uchar *data, ushort len )
{
	ushort	wp;
	ushort	size;
	PKT_RSND_BUFF	*cmd_buf;

	cmd_buf = &rsnd_add_cmd;

	size = len;
	while ((cmd_buf->total + len) > PKT_RSND_BUFF_SIZ) {		// overflow
		PKTbuf_DelReSendCommand();
	}

	wp = cmd_buf->wp;
	if ((wp+size) <= PKT_RSND_BUFF_SIZ) {
		memcpy(&cmd_buf->buff[wp], data, (size_t)size);
	} else {
		ushort	remain = PKT_RSND_BUFF_SIZ - wp;
		// �o�t�@�̏I�[�܂ŃZ�b�g
		memcpy(&cmd_buf->buff[wp], data, (size_t)remain);
		size -= remain;
		// �c��̓o�b�t�@�擪�ȍ~�ɃZ�b�g
		memcpy(&cmd_buf->buff[0], &data[remain], (size_t)size);
	}

	wp += len;
	if (wp >= PKT_RSND_BUFF_SIZ) {
		wp -= PKT_RSND_BUFF_SIZ;
	}
	cmd_buf->wp = wp;
	cmd_buf->total += len;
	cmd_buf->count++;

	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief			�R�}���h���đ��p�o�b�t�@���烊�[�h����
//[]----------------------------------------------------------------------[]
///	@param[in]		num		: ���Ԗڂ̃R�}���h
///	@param[out]		*data	: ���[�h�����R�}���h
///	@return			�R�}���h�T�C�Y(�R�}���h��+�R�}���h�f�[�^��)
///	@attention		�R�}���h�̓R�}���h��(2byte)�ƃR�}���h�f�[�^�ō\������܂��B
///	@note			�o�b�t�@�����O����Ă���R�}���h�̒��ōŌÂ̂��̂����<br>
///					���Ԗڂ̃R�}���h�����[�h����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort PKTbuf_ReadReSendCommand( ushort num, uchar *data )
{
	ushort	rp;
	ushort	size = 0, len;
	PKT_RSND_BUFF	*cmd_buf;
	ushort	count;

	cmd_buf = &rsnd_add_cmd;

	if( cmd_buf->count <= num ){
		return 0;
	}
	rp = cmd_buf->rp;
	for( count = 0; count <= num; count++ ){
		size = *(ushort*)&cmd_buf->buff[rp];
		size += 2;
		len = size;
		if (count == num) {
			if ((rp+size) <= PKT_RSND_BUFF_SIZ) {
				memcpy(data, &cmd_buf->buff[rp], (size_t)size);
			}
			else {
				ushort	remain = PKT_RSND_BUFF_SIZ - rp;
				// �o�t�@�̏I�[�܂œǂݏo��
				memcpy(data, &cmd_buf->buff[rp], (size_t)remain);
				// �c��̓o�b�t�@�擪����ǂݏo��
				memcpy(&data[remain], &cmd_buf->buff[0], (size_t)(size - remain));
			}
			break;
		}

		rp += len;
		if (rp >= PKT_RSND_BUFF_SIZ) {
			rp -= PKT_RSND_BUFF_SIZ;
		}
	}

	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			�đ��p�o�b�t�@���N���A����
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
///	@attention		
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/21<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PKTbuf_ClearReSendCommand( void )
{
	PKT_RSND_BUFF	*cmd_buf;

	cmd_buf = &rsnd_add_cmd;

// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:604)�Ή�
//	memset(&cmd_buf->count, 0, sizeof(PKT_RSND_BUFF));
	memset(cmd_buf, 0, sizeof(PKT_RSND_BUFF));
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:604)�Ή�
}

//[]----------------------------------------------------------------------[]
///	@brief			�đ����R�}���h���`�F�b�N����
//[]----------------------------------------------------------------------[]
///	@param[in]		none
///	@return			none
///	@attention		
///	@note			�đ��p�o�b�t�@�̐擪�ɃZ�b�g����Ă���R�}���hID�ƃT�u
///					�R�}���hID��Ԃ��܂�
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/25<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ulong PKTbuf_CheckReSendCommand( void )
{
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} stID;
	ulong	ID_SUBID;

	stID.uc.high = rsnd_add_cmd.buff[2];
	stID.uc.low  = rsnd_add_cmd.buff[3];
	ID_SUBID = stID.us << 16;

	stID.uc.high = rsnd_add_cmd.buff[4];
	stID.uc.low  = rsnd_add_cmd.buff[5];
// MH810100(S) S.Nishimoto 2020/04/08 �ÓI���(20200407:606)�Ή�
//	ID_SUBID |= stID.us;
	ID_SUBID |= (ulong)stID.us;
// MH810100(E) S.Nishimoto 2020/04/08 �ÓI���(20200407:606)�Ή�

	return ID_SUBID;
}

//[]----------------------------------------------------------------------[]
///	@brief			���M����R�}���h�f�[�^���o�b�t�@�ɃZ�b�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: �Z�b�g����R�}���h�f�[�^
///	@param[in]		len		: �R�}���h��
///	@return			TRUE	: OK<br>
///					FALSE	: NG
///	@attention		�o�b�t�@���I�[�o�[�t���[����Ƃ��͏����ݑΏۂ̃f�[�^��j������B
///	@note			���̃o�b�t�@�ɃZ�b�g���ꂽ�R�}���h�̓p�P�b�g�ʐM�^�X�N��<br>
///					���[�h���ăp�P�b�g�����܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
BOOL PKTbuf_SetSendCommand( const uchar *data, ushort len )
{
	int		i = 0;	// ���[�v�J�E���^
	ushort	wp;
	ushort	size;

	size = len;

	LockPKTSnd();

	// �o�b�t�@�ɓ��肫��Ȃ��H
	if( (snd_buf.total + len) > PKT_BUFF_SIZ ){
		// ���g���C����
		for( i=0; i<5; i++ ){
			// �o�b�t�@�ɓ��肫��Ȃ��H
			if( (snd_buf.total + len) > PKT_BUFF_SIZ ){
				queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );
				taskchg( IDLETSKNO );
			}else{
				break;
			}
		}
		if( i >= 5) {
			// �o�b�t�@�ɓ��肫��Ȃ��H
			if( (snd_buf.total + len) > PKT_BUFF_SIZ ){
				err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_BUF_FUL, 2, 0, 0, NULL );
				UnLockPKTSnd();
				return FALSE;		// overflow
			}
		}
	}

	if( PKT_BUFF_SIZ < snd_buf.wp ){	// ���M�o�b�t�@���C�g�|�C���^�ُ�
		// ���[�h/���C�g�|�C���^���N���A���č���̃R�}���h����������
		snd_buf.rp = 0;
		snd_buf.wp = 0;
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_BUF_WPT, 2, 0, 0, NULL );
	}
	wp = snd_buf.wp;
	wp %= PKT_BUFF_SIZ;

	// ���܂�H
	if( (wp+size) <= PKT_BUFF_SIZ ){
		memcpy( &snd_buf.buff[wp], data, (size_t)size );
	}else{
		ushort	remain = PKT_BUFF_SIZ - wp;
		// �o�t�@�̏I�[�܂ŃZ�b�g
		memcpy( &snd_buf.buff[wp], data, (size_t)remain );
		size -= remain;
		// �c��̓o�b�t�@�擪�ȍ~�ɃZ�b�g
		memcpy( &snd_buf.buff[0], &data[remain], (size_t)size );
	}

	wp += len;
	wp %= PKT_BUFF_SIZ;
	snd_buf.wp = wp;
	snd_buf.total += len;
	snd_buf.count++;
	
	UnLockPKTSnd();

	queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );

	return TRUE;
}

//[]----------------------------------------------------------------------[]
///	@brief			�R�}���h���o�b�t�@���烊�[�h����
//[]----------------------------------------------------------------------[]
///	@param[out]		*data	: ���[�h�����R�}���h
///	@param[in]		limit	: �����T�C�Y(0�`PKT_CMD_MAX)
///	@return			�R�}���h�T�C�Y(�R�}���h��+�R�}���h�f�[�^��)
///	@attention		�R�}���h�̓R�}���h��(2byte)�ƃR�}���h�f�[�^�ō\������܂��B
///	@note			�o�b�t�@�����O����Ă���R�}���h�̒��ōŌÂ̂��̂����[�h����B<br>
///					�R�}���h�������T�C�Y���傫���ꍇ�̓��[�h���Ȃ��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort PKTbuf_ReadSendCommand( uchar *data, ushort limit )
{
	ushort	rp;
	ushort	size, len;

	LockPKTSnd();

	// �o�b�t�@�ɃA�C�e���Ȃ��H
	if( snd_buf.count == 0 ){
		UnLockPKTSnd();
		return 0;
	}

	if( PKT_BUFF_SIZ < snd_buf.rp ){	// ���M�o�b�t�@���[�h�|�C���^�ُ�
		// ���[�h/���C�g�|�C���^���N���A���ēǂݍ��܂��ɖ߂�
		snd_buf.rp = 0;
		snd_buf.wp = 0;
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_BUF_RPT, 2, 0, 0, NULL );
		return 0;
	}
	rp = snd_buf.rp;
	size = *(ushort*)&snd_buf.buff[rp];
	size += 2;
	if( size > limit ){
		UnLockPKTSnd();
		return 0;
	}

	len = size;
	rp %= PKT_BUFF_SIZ;
	if( (rp+size) <= PKT_BUFF_SIZ ){
		memcpy( data, &snd_buf.buff[rp], (size_t)size );
	}
	else{
		ushort	remain = PKT_BUFF_SIZ - rp;
		// �o�t�@�̏I�[�܂œǂݏo��
		memcpy( data, &snd_buf.buff[rp], (size_t)remain );
		// �c��̓o�b�t�@�擪����ǂݏo��
		memcpy( &data[remain], &snd_buf.buff[0], (size_t)(size - remain) );
	}

	rp += len;
	rp %= PKT_BUFF_SIZ;
	snd_buf.rp = rp;
	snd_buf.total -= len;
	// ����̂Ńo�b�t�@����ɂȂ����H
	if( --snd_buf.count == 0 ){
		// buffer reset
		snd_buf.total = 0;
		snd_buf.rp = 0;
		snd_buf.wp = 0;
	}
	UnLockPKTSnd();

	return size;
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�����R�}���h�f�[�^���o�b�t�@�ɃZ�b�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: �Z�b�g����R�}���h�f�[�^
///	@param[in]		len		: �R�}���h��
///	@return			1:OK<br>
///					0:NG
///	@attention		�o�b�t�@���I�[�o�[�t���[����Ƃ��͏����ݑΏۂ̃f�[�^��j������B
///	@note			���̃o�b�t�@�ɃZ�b�g���ꂽ�R�}���h�̓��[����<br>
///					�I�y���[�V�����^�X�N�ɒʒm���܂��B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2009/09/25<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
BOOL PKTbuf_SetRecvCommand(const uchar *data, ushort len)
{
	ushort	wp;
	ushort	size;

	LockPKTRcv();

	size = len;
	if ((rcv_buf.total + 2 + len) > PKT_BUFF_SIZ) {
		UnLockPKTRcv();
		return 0;		// overflow
	}
	wp = rcv_buf.wp;
	*(ushort*)&rcv_buf.buff[wp] = size;		// �K���ۑ��ł���

	wp += 2;
	wp %= PKT_BUFF_SIZ;
	if ((wp+size) <= PKT_BUFF_SIZ) {
		memcpy(&rcv_buf.buff[wp], data, (size_t)size);
	} else {
		ushort	remain = PKT_BUFF_SIZ - wp;
		// �o�t�@�̏I�[�܂ŃZ�b�g
		memcpy(&rcv_buf.buff[wp], data, (size_t)remain);
		size -= remain;
		// �c��̓o�b�t�@�擪�ȍ~�ɃZ�b�g
		memcpy(&rcv_buf.buff[0], &data[remain], (size_t)size);
	}

	wp += len;
	wp %= PKT_BUFF_SIZ;
	rcv_buf.wp = wp;
	rcv_buf.total += (2+len);
	rcv_buf.count++;

	UnLockPKTRcv();

	return 1;
}


//[]----------------------------------------------------------------------[]
///	@brief			�O�������M�����R�}���h���o�b�t�@���烊�[�h����
//[]----------------------------------------------------------------------[]
///	@param[out]		*data	: ���[�h�����R�}���h
///	@return			�R�}���h�T�C�Y(�R�}���h��+�R�}���h�f�[�^��)
///	@attention		�I�y���[�V�����^�X�N�͎�M�ʒm�����[���Ŏ󂯂�x�Ƀ��[�h���邱�ƁB
///	@note			�o�b�t�@�����O����Ă���R�}���h�̒��ōŌÂ̂��̂����[�h����B
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2009/09/30<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
ushort PKTbuf_ReadRecvCommand(uchar *data)
{
	ushort	rp;
	ushort	size, len;

	LockPKTRcv();

	if (rcv_buf.count == 0) {
		UnLockPKTRcv();
		return 0;
	}
	rp = rcv_buf.rp;
	size = *(ushort*)&rcv_buf.buff[rp];
	len = size;
	rp += 2;
	rp %= PKT_BUFF_SIZ;
	if ((rp+size) <= PKT_BUFF_SIZ) {
		memcpy(data, &rcv_buf.buff[rp], (size_t)size);
	}
	else {
		ushort	remain = PKT_BUFF_SIZ - rp;
		// �o�t�@�̏I�[�܂œǂݏo��
		memcpy(data, &rcv_buf.buff[rp], (size_t)remain);
		// �c��̓o�b�t�@�擪����ǂݏo��
		memcpy(&data[remain], &rcv_buf.buff[0], (size_t)(size - remain));
	}
	rp += len;
	rp %= PKT_BUFF_SIZ;
	rcv_buf.rp = rp;
	rcv_buf.total -= (2 + len);
	if (--rcv_buf.count == 0) {
		// buffer reset
		rcv_buf.total =
		    rcv_buf.rp =
		        rcv_buf.wp = 0;
	}

	UnLockPKTRcv();

	return(size);
}

static void LockPKTSnd( void )
{
//	wai_sem(ID_PKT_SND_BUFF_SEM);
}

static void UnLockPKTSnd( void )
{
//	sig_sem(ID_PKT_SND_BUFF_SEM);
}

static void LockPKTRcv( void )
{
//	wai_sem(ID_PKT_RCV_BUFF_SEM);
}

static void UnLockPKTRcv( void )
{
//	sig_sem(ID_PKT_RCV_BUFF_SEM);
}
// MH810100(E) K.Onodera  2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
