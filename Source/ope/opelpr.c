/*[]----------------------------------------------------------------------[]*/
/*| ����������֘A����                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<stdio.h>
#include	<string.h>
#include	"system.h"
#include	"iodefine.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"prm_tbl.h"

/* A1�d���\���� */
typedef struct {
	uchar	code;			// �d�����ʺ���
	uchar	blank;			// ��
	uchar	reserve1;		// �\��1
	uchar	print_no;		// ���ް���
	uchar	format1;		// ̫�ϯđI��1
	uchar	format2;		// ̫�ϯđI��2
	uchar	format3;		// ̫�ϯđI��3
	uchar	format4;		// ̫�ϯđI��4
	uchar	data01[23];		// �ް�
	uchar	data02[15];
	uchar	data03[9];
	uchar	data04[9];
	uchar	data05[3];
	uchar	data06[10];
	uchar	data07[9];
	uchar	data08[23];
	uchar	data09[3];
	uchar	data10[3];
	uchar	data11[3];
	uchar	data12[7];
	uchar	data13[9];
	uchar	data14[7];
	uchar	data15[49];
	uchar	data16[25];
	uchar	data17[19];
	uchar	data18[9];
	uchar	data19[6];
	uchar	data20[9];
	uchar	data21[3];
	uchar	data22[10];
	uchar	data23[25];
	uchar	data24[3];
	uchar	data25[3];
	uchar	data26[3];
	uchar	data27[3];
	uchar	data28[23];
	uchar	data29[3];
	uchar	data30[3];
	uchar	data31[3];
	uchar	data32[27];
	uchar	data33[3];
	uchar	data34[3];
	uchar	data35[3];
	uchar	data36[27];
	uchar	data37[27];
	uchar	data38[1];
	uchar	reserve2[89];	// �\��2
} LprSndData_A1;

/* A1�d���p�Œ著�M������ */
static const uchar	LprString01[]	= "������Ԍ��i�J�[�h�p�j";
static const uchar	LprString01_2[]	= "�@�@�@�@�@�@�@�@�@�@�@";
static const uchar	LprString02[]	= "�@�@�@�^�@�@�@";
static const uchar	LprString02_2[]	= "�@�@�@�@�@�@�@";
static const uchar	LprString02_3[]	= "�@�e�@�X�@�g�@";
static const uchar	LprString03[]	= "No.     ";
static const uchar	LprString03_2[]	= "        ";
static const uchar	LprString04[]	= "�Ԏ��ԍ�";
static const uchar	LprString04_2[]	= "�@�@�@�@";
static const uchar	LprString05[]	= "��";
static const uchar	LprString05_3[]	= "�@";
static const uchar	LprString06[]	= "A01-0256L";
static const uchar	LprString06_2[]	= "         ";
static const uchar	LprString07[]	= "�L������";
static const uchar	LprString07_2[]	= "�@�@�@�@";
static const uchar	LprString08[]	= "�N�@�@�@�����܂Ł@�@�@";
static const uchar	LprString08_2[]	= "�N�@�@�@���@�@�@���܂�";
static const uchar	LprString08_3[]	= "�@�@�@�@�@�@�@�@�@�@�@";
static const uchar	LprString11[]	= "  ";
static const uchar	LprString12[]	= "�̎���";
static const uchar	LprString12_2[]	= "�@�@�@";
static const uchar	LprString13[]	= "  .  .  ";
static const uchar	LprString13_2[]	= "        ";
static const uchar	LprString14[]	= "�����O";
static const uchar	LprString14_2[]	= "�@�@�@";
static const uchar	LprString15[]	= "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@";
static const uchar	LprString16[]	= "�@������ԗp�X�e�b�J�[�@";
static const uchar	LprString17[]	= "�@�@�@�@�^�@�@�@�@";
static const uchar	LprString17_2[]	= "�@�@�e�@�X�@�g�@�@";
static const uchar	LprString18[]	= "����ԍ�";
static const uchar	LprString23[]	= "�L�������@�@�N�@�@������";
static const uchar	LprString23_2[]	= "�@�@�N�@�@���@�@������@";
static const uchar	LprString27[]	= "�N";
static const uchar	LprString28[]	= "�@�@�@�@�@�����܂Ł@�@";
static const uchar	LprString28_2[]	= "�@�@���@�@�@�@�@���܂�";
static const uchar	LprString32[]	= "���s���@�@�N�@�@���@�@���@";
static const uchar	LprString36[]	= "�@�@�@�@�@�@�@�@�@�@�@�@�@";
static const uchar	LprString32_1[]	= "���s���@�@�N�@�@���@�@����";

static const uchar	LprStrToku[]	= "��";
static const uchar	LprStrGaku[2][5]= {"���","�w��"};
static const uchar	LprStrSyu[6][7]	= {"���]��","�o�C�N","���t�@","�����","����","����"};


/* ������������M�p�ޯ̧ */
static struct {
	ushort	size;
	uchar	buf[508];
} LprSndBuf;

/* static functions prototype */
static void	LprErrChk(uchar code, uchar onoff);

/*[]----------------------------------------------------------------------[]*/
/*| ���M�ޯ̧������                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpLpr_init( void )                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpLpr_init(void)
{
	memset(&LprSndBuf, 0, sizeof(LprSndBuf));
}

/*[]----------------------------------------------------------------------[]*/
/*| ������������䏈��                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpLpr_ctl( ret )                                        |*/
/*| PARAMETER    : msg  : �����                                            |*/
/*|              : data : �ް�                                             |*/
/*| RETURN VALUE : �����                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	OpLpr_ctl(ushort msg, uchar *data)
{
	switch (msg){
	case IBK_LPR_SRLT_REC:							// ���M�����ް���M
		if (data[0] != 0) {	/* ���M����NG�H */
			if (LprSndBuf.size) {
			}
		}
		break;

	case IBK_LPR_ERR_REC:							// �װ�ް���M
		/* �װ����ެ��ُo�͂��� */
		LprErrChk( data[1], data[2] );
		break;

	case IBK_LPR_B1_REC:							// �󎚏I���ʒm��M
		/* �󎚏I����� �m�F */
		if (data[4] == 0x30) {
			msg = IBK_LPR_B1_REC_OK;				// ����Đ���
		}
		else {
			msg = IBK_LPR_B1_REC_NG;				// ����Ď��s
			LprErrChk( data[4], 1 );
		}
		break;

	default:
		break;
	}
	
	return msg;
}

/*[]----------------------------------------------------------------------[]*/
/*| �������������ޑ��M����(�d�����ʺ���=A1h�Œ�)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : OpLpr_snd( code )                                       |*/
/*| PARAMETER    : tcode : ����l����                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : ART ogura                                               |*/
/*| Date         : 2005-10-31                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	OpLpr_snd( Receipt_data *lpn, char test_flg )
{
	LprSndData_A1	*p;
	uchar			uc_tksyu;
	uchar			uc_toku, uc_gaku, uc_syu;
	uchar			uc_form1, uc_form2, uc_form3, uc_form4;

	OpLpr_init();													// �������������Ӽޭ�ُ�����

	LprSndBuf.size = 508;

	/* ���M�ް��쐬(�ް����̂�) */
	p = (LprSndData_A1 *)&LprSndBuf.buf[8];

	p->code = 0xA1;													// �d�����ʺ���
	p->blank = 0;													// ��
	p->reserve1 = 0;												// �\��1

	uc_tksyu = (uchar)(lpn->teiki.syu - 1);
	uc_toku = (uchar)prm_get( COM_PRM,S_KOU,(short)(33+10*uc_tksyu),1,3 );	// ���p���(����)
	uc_gaku = (uchar)prm_get( COM_PRM,S_KOU,(short)(33+10*uc_tksyu),1,2 );	// ���p���(�w��)
	uc_syu  = (uchar)prm_get( COM_PRM,S_KOU,(short)(33+10*uc_tksyu),1,1 );	// ���p���(�Ԏ�)

	uc_form1 = (uchar)prm_get( COM_PRM,S_KOU,27,1,3 );				// ̫�ϯđI��1�ݒ�
	uc_form2 = (uchar)prm_get( COM_PRM,S_KOU,27,1,2 );				// ̫�ϯđI��2�ݒ�
	uc_form3 = (uchar)prm_get( COM_PRM,S_KOU,27,1,1 );				// ̫�ϯđI��3�ݒ�
	uc_form4 = (uchar)prm_get( COM_PRM,S_KOU,26,1,2 );				// ̫�ϯđI��4�ݒ�

	p->format1 = uc_form1;											// ̫�ϯđI��1
	p->format2 = uc_form2;											// ̫�ϯđI��2
	p->format3 = uc_form3;											// ̫�ϯđI��3
	p->format4 = 0;													// ̫�ϯđI��4

	if( CPrmSS[S_KOU][23] ){										// �p���ݒ�2�����s
		// 2�����s�ݒ莞�Ͷ��ޓ\��t���p�ް����쐬
		p->print_no = 38;											// ���ް���
		memcpy( p->data01, LprString01, sizeof(p->data01) );		// "������Ԍ��i�J�[�h�p�j"
		if( test_flg == 1 ){
			memcpy( p->data02, LprString02_3, sizeof(p->data02) );	// "�@�e�@�X�@�g�@"
		}else{
			memcpy( p->data02, LprString02, sizeof(p->data02) );	// "�@�@�@�^�@�@�@"
			if( uc_toku ){											// ����
				memcpy( p->data02, LprStrGaku[uc_gaku], 4 );		// "���","�w��"
				memcpy( &p->data02[4], LprStrToku, 2 );				// "��"
			}else{
				memcpy( &p->data02[2], LprStrGaku[uc_gaku], 4 );	// "���","�w��"
			}
			memcpy( &p->data02[8], LprStrSyu[uc_syu], 6 );			// "���]��","�o�C�N","���t�@","�����","����","����"
		}
		memcpy( p->data03, LprString03, sizeof(p->data03) );		// "No.     "
		intoasl( &p->data03[3], lpn->teiki.id, 5 );					// �����ID

		if( uc_form3 == 0 ){
			// �Ԏ��ԍ��󎚂��Ȃ�
			memcpy( p->data04, LprString04_2, sizeof(p->data04) );	// "�@�@�@�@"
			memcpy( p->data05, LprString05_3, sizeof(p->data05) );	// "�@"
			memcpy( p->data06, LprString06_2, sizeof(p->data06) );	// "         "
		}else{
			// �Ԏ��ԍ��󎚂���
			memcpy( p->data04, LprString04, sizeof(p->data04) );	// "�Ԏ��ԍ�"
			memcpy( p->data05, LprString05, sizeof(p->data05) );	// "��"			�_�~�[
			memcpy( p->data06, LprString06, sizeof(p->data06) );	// "A01-0256L"	�_�~�[
		}

		memcpy( p->data07, LprString07, sizeof(p->data07) );		// "�L������"

		if( uc_form2 == 0 ){
			// xx�����܂�
			memcpy( p->data08, LprString08, sizeof(p->data08) );	// "�N�@�@�@�����܂�"
			memcpy( p->data11, LprString11, sizeof(p->data11) );	// "  "���ͽ�߰��Ŗ��߂�
		}else{
			// xx��xx���܂�
			memcpy( p->data08, LprString08_2, sizeof(p->data08) );	// "�N�@���@���܂Ł@"
			intoas( p->data11, (ushort)(lpn->teiki.e_day), 2 );		// ��
		}
		intoas( p->data09, (ushort)(lpn->teiki.e_year%100), 2 );	// �N
		intoas( p->data10, (ushort)(lpn->teiki.e_mon), 2 );			// ��

		memcpy( p->data12, LprString12, sizeof(p->data12) );		// "�̎���"
		memcpy( p->data13, LprString13, sizeof(p->data13) );		// "  .  .  "
		intoas( &p->data13[0], (ushort)(lpn->TOutTime.Year%100), 2 );	// �N
		intoas( &p->data13[3], (ushort)(lpn->TOutTime.Mon), 2 );	// ��
		intoas( &p->data13[6], (ushort)(lpn->TOutTime.Day), 2 );	// ��
		memcpy( p->data14, LprString14, sizeof(p->data14) );		// "�����O"

		memcpy( p->data15, LprString15, sizeof(p->data15) );		// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
		if( uc_form4 == 1 ){										// �w�b�_�[�t�@�C����p����
			// �̎��؂�ͯ�ް�𗬗p���Ē��֏ꖼ�͉ςɐݒ�ł���悤�ɂ���B
			// �̎��؂�ͯ�ް(Header.txt)��3�A4�s�ڂ����x���p�Ƃ��Ďg�p����
			// ���قɈ󎚂ł��镶������24����(48�޲�)�B
			// ͯ�ް��1�s36�޲ĂȂ̂ŁAͯ�ް��3�s�ڂ̐擪����24�޲Ă�4�s�ڐ擪����24�޲Ă��g�p����B
			// ͯ�ް̧�ق�3�s�ځA4�s�ڂƂ��S�p13�����ڂ͑S�p��߰��Ƃ��鎖�B
			// �S�đS�p�Őݒ肷�鎖�B
			memcpy( p->data15, &Header_Data[2][0], 24 );
			memcpy( &p->data15[24], &Header_Data[3][0], 24 );
		}
	}else{
		// 1�����s�ݒ莞�Ͷ��ޓ\��t���p�ް��͂��ׂăX�y�[�X�Ŗ��߂�
		p->print_no = 37;											// ���ް���
		memcpy( p->data01, LprString01_2, sizeof(p->data01) );		// "�@�@�@�@�@�@�@�@�@�@�@"
		memcpy( p->data02, LprString02_2, sizeof(p->data02) );		// "�@�@�@�@�@�@�@"
		memcpy( p->data03, LprString03_2, sizeof(p->data03) );		// "        "
		memcpy( p->data04, LprString04_2, sizeof(p->data04) );		// "�@�@�@�@"
		memcpy( p->data05, LprString05_3, sizeof(p->data05) );		// "�@"
		memcpy( p->data06, LprString06_2, sizeof(p->data06) );		// "         "
		memcpy( p->data07, LprString07_2, sizeof(p->data07) );		// "�@�@�@�@"
		memcpy( p->data08, LprString08_3, sizeof(p->data08) );		// "�@�@�@�@�@�@�@�@�@�@�@"
		memcpy( p->data09, LprString11, sizeof(p->data09) );		// "  "
		memcpy( p->data10, LprString11, sizeof(p->data10) );		// "  "
		memcpy( p->data11, LprString11, sizeof(p->data11) );		// "  "
		memcpy( p->data12, LprString12_2, sizeof(p->data12) );		// "�@�@�@"
		memcpy( p->data13, LprString13_2, sizeof(p->data13) );		// "        "
		memcpy( p->data14, LprString14_2, sizeof(p->data14) );		// "�@�@�@"
		memcpy( p->data15, LprString15, sizeof(p->data15) );		// "�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@"
	}

	// �ԑ̓\��t���p�ް��쐬
	memcpy( p->data16, LprString16, sizeof(p->data16) );			// "������ԗp�X�e�b�J�["
	memcpy( p->data17, LprString17, sizeof(p->data17) );			// "�@�w�����^���]�ԁ@"

	if( test_flg == 1 ){
		memcpy( p->data17, LprString17_2, sizeof(p->data17) );		// "�@�@�e�@�X�@�g�@�@"
	}else{
		memcpy( p->data17, LprString17, sizeof(p->data17) );		// "�@�@�@�@�^�@�@�@�@"
		if( uc_toku ){												// ����
			memcpy( &p->data17[2], LprStrGaku[uc_gaku], 4 );		// "���","�w��"
			memcpy( &p->data17[6], LprStrToku, 2 );					// "��"
		}else{
			memcpy( &p->data17[4], LprStrGaku[uc_gaku], 4 );		// "���","�w��"
		}
		memcpy( &p->data17[10], LprStrSyu[uc_syu], 6 );				// "���]��","�o�C�N","���t�@","�����","����","����"
	}

	memcpy( p->data18, LprString18, sizeof(p->data18) );			// "����ԍ�"
	intoasl( p->data19, lpn->teiki.id, 5 );							// �����ID

	if( uc_form3 == 0 ){
		// �Ԏ��ԍ��󎚂��Ȃ�
		memcpy( p->data20, LprString04_2, sizeof(p->data20) );		// "�@�@�@�@"
		memcpy( p->data21, LprString05_3, sizeof(p->data21) );		// "�@"
		memcpy( p->data22, LprString06_2, sizeof(p->data22) );		// "         "
	}else{
		// �Ԏ��ԍ��󎚂���
		memcpy( p->data20, LprString04, sizeof(p->data20) );		// "�Ԏ��ԍ�"
		memcpy( p->data21, LprString05, sizeof(p->data21) );		// "��"			�_�~�[
		memcpy( p->data22, LprString06, sizeof(p->data22) );		// "A01-0256L"	�_�~�[
	}

	if( uc_form1 == 0 ){
		// �L������xx�Nxx������
		memcpy( p->data23, LprString23, sizeof(p->data23) );		// "�L�������@�@�N�@�@������"
		memcpy( p->data26, LprString11, sizeof(p->data26) );		// "  "���ͽ�߰��Ŗ��߂�
	}else{
		// xx�Nxx��xx������
		memcpy( p->data23, LprString23_2, sizeof(p->data23) );		// "�@�@�N�@�@���@�@������@"
		intoas( p->data26, (ushort)(lpn->teiki.s_day), 2 );			// ��
	}
	intoas( p->data24, (ushort)(lpn->teiki.s_year%100), 2 );		// �N
	intoas( p->data25, (ushort)(lpn->teiki.s_mon), 2 );				// ��

	memcpy( p->data27, LprString27, sizeof(p->data27) );			// "�N"
	if( uc_form2 == 0 ){
		// xx�����܂�
		memcpy( p->data28, LprString28, sizeof(p->data28) );		// "�@�@�@�@�@�����܂Ł@�@"
		memcpy( p->data31, LprString11, sizeof(p->data31) );		// "  "���ͽ�߰��Ŗ��߂�
	}else{
		// xx��xx���܂�
		memcpy( p->data28, LprString28_2, sizeof(p->data28) );		// "���@�@�@�@�@���܂Ł@�@"
		intoas( p->data31, (ushort)(lpn->teiki.e_day), 2 );			// ��
	}
	intoas( p->data29, (ushort)(lpn->teiki.e_year%100), 2 );		// �N
	intoas( p->data30, (ushort)(lpn->teiki.e_mon), 2 );				// ��

	if( test_flg == 2 ){
		memcpy( p->data32, LprString32_1, sizeof(p->data32) );			// "���s���@�@�N�@�@���@�@����"
	}else{
		memcpy( p->data32, LprString32, sizeof(p->data32) );			// "���s���@�@�N�@�@���@�@��"
	}
	intoas( p->data33, (ushort)(lpn->TOutTime.Year%100), 2 );		// �N
	intoas( p->data34, (ushort)(lpn->TOutTime.Mon), 2 );			// ��
	intoas( p->data35, (ushort)(lpn->TOutTime.Day), 2 );			// ��

	memcpy( p->data36, LprString36, sizeof(p->data36) );			// "�@�@�@�@�@�@�@�@�@�@�@�@�@"
	memcpy( p->data37, LprString36, sizeof(p->data37) );			// "�@�@�@�@�@�@�@�@�@�@�@�@�@"
	if( uc_form4 == 1 ){											// �w�b�_�[�t�@�C����p����?
		// �̎��؂�ͯ�ް�𗬗p���Ē��֏ꖼ�͉ςɐݒ�ł���悤�ɂ���B
		// �̎��؂�ͯ�ް(Header.txt)��3�A4�s�ڂ����x���p�Ƃ��Ďg�p����
		// ���قɈ󎚂ł��镶������13����(26�޲�)�~2�s�B
		// ͯ�ް��1�s36�޲ĂȂ̂ŁAͯ�ް��3�s�ڂ̐擪����24�޲Ă�4�s�ڐ擪����24�޲Ă��g�p����B
		// ͯ�ް̧�ق�3�s�ځA4�s�ڂƂ��S�p13�����ڂ͑S�p��߰��Ƃ��鎖�B
		// �S�đS�p�Őݒ肷�鎖�B
		memcpy( p->data36, &Header_Data[2][0], 24 );
		memcpy( p->data37, &Header_Data[3][0], 24 );
	}

	/* ���M */
}

/*[]----------------------------------------------------------------------[]*/
/*| ���ي֘A�װ�o�^����                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : LprErrChk( code, onoff )                                |*/
/*| PARAMETER    : code   : �װ/�װѺ���                                   |*/
/*|              : onoff  : 0=���� 1=����                                  |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2006-03-17                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
static void	LprErrChk(uchar code, uchar onoff)
{
	uchar	err_arm;

	switch (code) {
	/* �װ���� */
	case 0x01:		/* IBK�v�����^���W���[�� - �v�����^�ԒʐM�^�C���A�E�g */
		code = 1;
		err_arm = 1;
		break;
	case 0x07:		/* ���x���v�����^�@�I�����C���^�I�t���C����� */
	case 0x34:		/* �I�t���C�� */
		code = 20;
		err_arm = 1;
		break;
	case 0x61:		/* �o�b�t�@�I�[�o�[ */
		code = 14;
		err_arm = 1;
		break;
	case 0x62:		/* �w�b�h�I�[�v�� */
		code = 11;
		err_arm = 1;
		break;
	case 0x65:		/* ���f�B�A�G���[ */
		code = 15;
		err_arm = 1;
		break;
	case 0x66:		/* �Z���T�G���[ */
		code = 13;
		err_arm = 1;
		break;
	case 0x67:		/* �w�b�h�G���[ */
		code = 12;
		err_arm = 1;
		break;
	case 0x6a:		/* �J�b�^�G���[ */
		code = 10;
		err_arm = 1;
		break;
	case 0x6b:		/* ���̑��̃G���[ */
		code = 16;
		err_arm = 1;
		break;

	/* �װѺ��� */
	case 0x63:		/* �y�[�p�[�G���h */
		code = 46;
		err_arm = 2;
		break;
	case 0x64:		/* ���{���G���h */
		code = 47;
		err_arm = 2;
		break;
	case 0x31:		/* ���{���j�A�G���h */
		code = 49;
		err_arm = 2;
		break;
	default:
		err_arm = 0;
		break;
	}

	if( err_arm == 1 ){
		err_chk( ERRMDL_LABEL, (char)code, (char)onoff, 0, 0 );	// ���ٴװ�o�^
	}
	else if( err_arm == 2 ){
		alm_chk( ALMMDL_SUB, (char)code, (char)onoff );			// ���ٱװѓo�^
	}
}
