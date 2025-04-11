#ifndef	RAUIDPROC_H
#define	RAUIDPROC_H
/****************************************************************************/
/*																			*/
/****************************************************************************/


/****************************************************************/
/*		�f�[�^�e�[�u�����\����								*/
/****************************************************************/

#define	REQUEST_TABLE_MAX	15			/* �ʐM�v���f�[�^�E�e�[�u���N���A�v���f�[�^�ŗv�������ő�e�[�u���� */


#if 1
typedef struct {
	uchar		*pc_AreaTop;			// �̈�擪�A�h���X
	ulong		ul_AreaSize;			// �̈�T�C�Y
	ulong		ul_NearFullMaximum;		// �j�A�t���A���[������l
	ulong		ul_NearFullMinimum;		// �j�A�t���A���[�������l
	ushort		i_NearFullStatus;		// �j�A�t���������
	ushort		i_NearFullStatusBefore;	// �j�A�t��������ԁi�O��j
	uchar		*pc_ReadPoint;			// �f�[�^�ǂݍ��݈ʒu
	//ulong		ul_ReadPoint;			// �f�[�^�ǂݍ��݈ʒu
	uchar		*pc_WritePoint;			// �f�[�^�������݈ʒu
	//ulong		ul_WritePoint;			// �f�[�^�������݈ʒu
	ushort		ui_DataCount;			// �f�[�^����
	ushort		ui_SendDataCount;		// ���M�Ώۃf�[�^����
	char		c_DataFullProc;			// �o�b�t�@�t�����f�[�^�����@
	uchar		uc_DataId;
	ushort		ui_syuType;				// �W�v�^�C�v
} RAU_DATA_TABLE_INFO;
#endif

/*------------------------���M�Ώۏ��----------------------------------------*/
//--- �o�b�t�@���Ƃ̑��M��� ---
typedef struct {
	uchar	send_req;			// ���M�ΏۂƂ��Ďw�肳�ꂽ���ǂ���
	ushort	send_data_count;	// ���M�Ώی���
	ushort	send_complete_count;// ���M��������
	ushort	fill_up_data_count;	// �[�U�����f�[�^��
	ushort	crc_err_data_count;	// CRC�G���[�������f�[�^��
	RAU_DATA_TABLE_INFO *pbuff_info;
} T_SEND_DATA_ID_INFO;

//--- ���M���̃o�b�t�@���̔z�� ---
typedef struct {
	T_SEND_DATA_ID_INFO send_data_id_info[17];
} T_SEND_DATA_INFO;

//--- Dopa���M�pNT-NET�f�[�^��� ---
typedef struct {
	uchar	*nt_data_len;		// NT-NET�f�[�^���ʒu
	uchar	*nt_data_start;		// NT-NET�f�[�^�J�n�ʒu
	uchar	*nt_data_end;		// NT-NET�f�[�^�I���ʒu
	uchar	*nt_data_crc;		// NT-NET CRC  �I���ʒu
	uchar	*nt_data_next;		// NT-NET�f�[�^����ǂݍ��݈ʒu
} T_SEND_NT_DATA;


//--- T���v�@���K�Ǘ��@NT-NET�f�[�^��� ---
/* NT buffer structure */
struct _ntbuf {
	char	*next;
	long	length;
};
#define	_GET_DATA(bf)	((char*)bf+sizeof(struct _ntbuf))

/* NT buffer management structure */
struct _ntman {
	char	*top;
	char	*bottom;
	char	*freearea;
	char	*sendtop;
	char	*sendnow;
};

#define	ALIGN(n)	((n+1)&0xfffffffe)


/****************************************************************/
/*		�[�����M�p�L���[										*/
/****************************************************************/
#define	TERM_QUE_MAX	1

/****************************************************************/
/*		�g�n�r�s���M�p�L���[�i�[������X���[)					*/
/****************************************************************/
#define	HOST_QUE_MAX	1

/****************************************************************/
/*		�g�n�r�s���M�p�L���[�i�e�[�u���f�[�^)					*/
/****************************************************************/
#define	HOST_TABLE_QUE_MAX	1

typedef struct _host_table_no {
	ushort		ui_rec_old_bk;							/* �ŌÃ��R�[�h�ԍ��o�b�N�A�b�v			*/
	ushort		ui_rec_new_bk;							/* �ŐV���R�[�h�ԍ��o�b�N�A�b�v			*/
	ushort		ui_rec_now_bk;							/* �����R�[�h���o�b�N�A�b�v				*/
	ushort		ui_rec_y_old_bk;						/* �����M�ŌÃ��R�[�h�ԍ��o�b�N�A�b�v	*/
	ushort		ui_rec_y_now_bk;						/* �����M���R�[�h���o�b�N�A�b�v			*/
	ushort		ui_rec_no_bk;							/* �L���[�̃f�[�^�̍Ō�̃��R�[�h��		*/
	ushort		ui_rec_cnt_bk;							/* �L���[�̃f�[�^�̃��R�[�h��			*/
	ulong		ul_rec_tbl_no_bk;						/* �L���[�̃f�[�^�̃e�[�u���f�[�^��		*/
} HOST_TABLE_NO;

/****************************************************************/
/*		���M�쐬�҂��d��ID�L���[								*/
/****************************************************************/
#define	RAU_ID_QUE_MAX_T	1
#define	RAU_ID_QUE_MAX_H	1

/****************************************************************/
/*		�q�`�t�ݒ�d���\����									*/
/****************************************************************/
typedef struct _RAU_CONFIG {
	char		proc_kind[2];			/* �����敪 */
	char		parking_id[8];			/* ���ԏ�h�c */
	char		password[8];			/* �p�X���[�h */
	char		host_tel1[14];			/* �z�X�g���s�d�k�ԍ��P */
	char		host_tel2[14];			/* �z�X�g���s�d�k�ԍ��Q */
	char		host_tel3[14];			/* �z�X�g���s�d�k�ԍ��R */
	char		id22_23_tel_flg;		/* �h�c�������M�t���O  �i���Z�f�[�^�j*/
	char		id22_23_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i���Z�f�[�^�j*/
	char		id30_41_tel_flg;		/* �h�c�������M�t���O  �i�W�v�f�[�^�j*/
	char		id30_41_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i�W�v�f�[�^�j*/
	char		id121_tel_flg;			/* �h�c�������M�t���O  �i�A���[���f�[�^�j*/
	char		id121_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i�A���[���f�[�^�j*/
	char		id123_tel_flg;			/* �h�c�������M�t���O  �i���샂�j�^�[�f�[�^�j*/
	char		id123_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i���샂�j�^�[�f�[�^�j*/
	char		id120_tel_flg;			/* �h�c�������M�t���O  �i�G���[�f�[�^�j*/
	char		id120_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i�G���[�f�[�^�j*/
	char		id20_21_tel_flg;		/* �h�c�������M�t���O  �i���ɏo�Ƀf�[�^�j*/
	char		id20_21_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i���ɏo�Ƀf�[�^�j*/
	char		id122_tel_flg;			/* �h�c�������M�t���O  �i���j�^�[�f�[�^�j*/
	char		id122_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i���j�^�[�f�[�^�j*/
	char		id131_tel_flg;			/* �h�c�������M�t���O  �i�R�C�����ɏW�v�f�[�^�j*/
	char		id131_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i�R�C�����ɏW�v�f�[�^�j*/
	char		id133_tel_flg;			/* �h�c�������M�t���O  �i�������ɏW�v�f�[�^�j*/
	char		id133_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i�������ɏW�v�f�[�^�j*/
	char		id236_tel_flg;			/* �h�c�������M�t���O  �i���ԑ䐔�f�[�^�j*/
	char		id236_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i���ԑ䐔�f�[�^�j*/
	char		id237_tel_flg;			/* �h�c�������M�t���O  �i���䐔�E���ԃf�[�^�j*/
	char		id237_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i���䐔�E���ԃf�[�^�j*/
	char		id126_tel_flg;			/* �h�c�������M�t���O  �i���K�Ǘ��f�[�^�j*/
	char		id126_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i���K�Ǘ��f�[�^�j*/
	char		id135_tel_flg;			/* �h�c�������M�t���O  �i�ޑK�Ǘ��W�v�f�[�^�j*/
	char		id135_tel_no;			/* �h�c�������M�s�d�k�ԍ�  �i�ޑK�Ǘ��W�v�f�[�^�j*/
	char		yobi[8];				/* �\�� */
	char		tel_priority[3];		/* ���M��D�揇��  */
	char		speed;					/* �ʐM���x	 */
	char		data_len;				/* �f�[�^�� */
	char		stop_bit;				/* �X�g�b�v�r�b�g */
	char		parity_bit;				/* �p���e�B�r�b�g */
	char		deta_full_proc;			/* �f�[�^�������@ */
	char		retry[2];				/* ���g���C�� */
	char		time_out[2];			/* �^�C���A�E�g */
	char		modem_retry;			/* ���f���ڑ����g���C */
	char		point_change;			/* �ړ_�ω��������M */
	char		memory_alarm;			/* �������A���[�����M */
	char		rau_check_sum[4];		/* �q�`�t�`�F�b�N�T�� */
	char		rau_version[8];			/* �q�`�t�o�[�W���� */
	char		init_data[64];			/* �������f�[�^ */
	uchar		IBW_RespWtime[2];		/* ���M���ʎ�M�����҂����ԁi�P�ʁ��b�j*/
										/* �o�C�i���Ŋi�[ [0]=High byte, [1]=Low byte */
										/* ��j0x4E20 => [0]=0x4E, [1]=0x20 */
	uchar		system_select;			/* �V�X�e���I�� */
	uchar		modem_exist;			/* ���f���̗L�� */
	uchar		foma_dopa;				/* 20H/'0' = DOPA   / '1' = FOMA, FOMA�̏ꍇ init_data �� APN�w�� ex>' 1sf.amano.co.jp' */
	uchar		yobi2[7];				/* �\�� */
	uchar		mobile_ark;				/* MobileArk�I�� */
	uchar		Dpa_proc_knd;			/* Dopa �����敪 [0]=�펞�ʐM�Ȃ�, [1]=�펞�ʐM���� */
	uchar		Dpa_IP_h[12];			/* Dopa HOST��IP�A�h���X */
	uchar		Dpa_port_h[5];			/* Dopa HOST�ǃ|�[�g�ԍ� */
	uchar		Dpa_IP_m[12];			/* Dopa ����IP�A�h���X */
	uchar		Dpa_port_m[5];			/* Dopa ���ǃ|�[�g�ԍ� */
	uchar		Dpa_ppp_ninsho;			/* Dopa ���M���F�؎菇 */
	uchar		Dpa_nosnd_tm[4];		/* Dopa ���ʐM�^�C�}�[(�b) */
	uchar		Dpa_cnct_rty_tm[3];		/* Dopa �Ĕ��đ҂��^�C�}�[(�b) */
	uchar		Dpa_com_wait_tm[3];		/* Dopa �R�}���h(ACK)�҂��^�C�}�[(�b) */
	uchar		Dpa_data_rty_tm[3];		/* Dopa �f�[�^�đ��҂��^�C�}�[(��) */
	uchar		Dpa_discnct_tm[4];		/* Dopa TCP�R�l�N�V�����ؒf�҂��^�C�}�[(�b) */
	uchar		Dpa_cnct_rty_cn[3];		/* Dopa �Ĕ��ĉ� */
	uchar		Dpa_data_rty_cn;		/* Dopa �f�[�^�đ���(NAK,������) */
	uchar		Dpa_data_code;			/* Dopa HOST���d���R�[�h [0]=�����R�[�h, [1]=�o�C�i���R�[�h */
	uchar		yobi3[2];				/* �\��3 */

	uchar		alarm_non_notify[60];	/* �A���[����ʒm�e�[�u���f�[�^�P�`�R�O */
	uchar		error_non_notify[60];	/* �G���[��ʒm�@�e�[�u���f�[�^�P�`�R�O */
	uchar		interrupt_proc;			/* ���f�v�����̏��� */
	uchar		interrupt_proc_block[2];/* ���f�v����M���̔���u���b�N */

	uchar		Dpa_data_snd_rty_cnt;	// �c���o���������p�P�b�g���M���g���C��(������)
	uchar		Dpa_ack_wait_tm[3];		// �c���o���������p�P�b�g�����҂�����(�`�b�j/�m�`�j)
	uchar		Dpa_data_rcv_rty_cnt;	// �c���o���������p�P�b�g��M���g���C��(�m�`�j)
	uchar		Dpa_port_watchdog_tm[3];// �c���o���������ʐM�Ď��^�C�}(�b)
	uchar		Dpa_port_m2[5];			// �c���o�������� ���ǃ|�[�g�ԍ�
	uchar		netmask[2];				// �w�o�������T�u�l�b�g�}�X�N
	uchar		gateway[12];			// �f�t�H���g�Q�[�g�E�F�C

	uchar		yobi4[95];				/* �\��3 */
} RAU_CONFIG;

/****************************************************************/
/*		�c�������q�`�t�ݒ�d���\����							*/
/****************************************************************/
typedef struct _DPA_RAU_CONFIG {
	uchar		rau_ip[12];				// ���ǂh�o�A�h���X
	uchar		rau_port[5];			// �y������z���ǃ|�[�g�ԍ�
	uchar		host_ip[12];			// �y������z�g�n�r�s�ǂh�o�A�h���X
	uchar		host_port[5];			// ���u�g�n�r�s�ǃ|�[�g�ԍ�
	uchar		ninsho;					// �o�o�o�F�؎菇
	uchar		rev_user_id[21];		// �g�n�r�s�ǂo�o�o�F�؂h�c
	uchar		rev_password[15];		// �g�n�r�s�ǂo�o�o�F�؃p�X���[�h
	uchar		send_user_id[21];		// ���ǂo�o�o�F�؂h�c
	uchar		send_password[15];		// ���ǂo�o�o�F�؃p�X���[�h
	uchar		Dpa_proc_knd;			// �c���o�������敪('0':�펞�ʐM�Ȃ�, '1':�펞�ʐM����)
	uchar		no_send_time[4];		// ���ʐM�^�C�}�[(�b)
	uchar		dail_retry_time[3];		// �Ĕ��đ҂��^�C�}�[(�b)
	uchar		command_wait_time[3];	// �y�ナ����z�p�P�b�g�����҂����ԁi�`�b�j�^�m�`�j�j
	uchar		connect_retry[3];		// �Ĕ��ĉ�
	uchar		data_retry;				// �y�ナ����z�p�P�b�g���M���g���C�񐔁i�������j
	uchar		data_retry_time[3];		// �f�[�^�đ��҂��^�C�}�[(��)
	uchar		disconect_time[4];		// �s�b�o�R�l�N�V�����ؒf�҂��^�C�}�[(�b)
	uchar		Dpa_data_code;			// �g�n�r�s���d���R�[�h('0':�����R�[�h, '1':�o�C�i���R�[�h)
	uchar		data_snd_rty;			// �y��������z�p�P�b�g���M���g���C�񐔁i�������j
	uchar		ack_wait_tim[3];		// �y��������z�p�P�b�g�����҂����ԁi�`�b�j�^�m�`�j�j
	uchar		data_rcv_rty;			// �y��������z�p�P�b�g��M���g���C�񐔁i�m�`�j�j
	uchar		port_watchdog_tim[3];	// �y��������z��M�Ď��^�C�}�[
	uchar		rau_port_m2[5];			// �y��������z���ǃ|�[�g�ԍ�
	uchar		netmask[2];				// �w�o�������T�u�l�b�g�}�X�N
	uchar		yobi[5];				// �\��
	uchar		dopa_cre_proc_kind;		// �N���W�b�g�c���o�������敪
	uchar		cre_ip[12];				// �N���W�b�g���ǂh�o�A�h���X
	uchar		cre_port[5];			// �N���W�b�g���ǃ|�[�g�ԍ�
	uchar		cre_host_ip[12];		// �N���W�b�g�g�n�r�s�ǂh�o�A�h���X
	uchar		cre_host_port[5];		// �N���W�b�g�g�n�r�s�ǃ|�[�g�ԍ�
	uchar		cre_data_retry[2];		// �N���W�b�g�p�P�b�g���M���g���C��
	uchar		cre_data_retry_time[2];	// �N���W�b�g�p�P�b�g�����҂�����
} DPA_RAU_CONFIG;

/////////////////////////////////////////////////////////////////
//			�e�[�u�������f�[�^�\����
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking[2];		// ���Ƀf�[�^			�iID20�j
	uchar	out_parking[2];		// �o�Ƀf�[�^			�iID21�j
	uchar	paid_data[2];		// ���Z�f�[�^			�iID22�j	�����ł�22�@23�@�����Z����
	uchar	total[2];			// T���v�W�v�f�[�^		�iID30�`38�E41�j
	uchar	error[2];			// �G���[�f�[�^			�iID120�j
	uchar	alarm[2];			// �A���[���f�[�^		�iID121�j
	uchar	monitor[2];			// ���j�^�[�f�[�^		�iID122�j
	uchar	ope_monitor[2];		// ���샂�j�^�[�f�[�^	�iID123�j
	uchar	coin_total[2];		// �R�C�����ɏW�v�f�[�^	�iID131�j
	uchar	money_toral[2];		// �������ɏW�v�f�[�^	�iID133�j
	uchar	parking_num[2];		// ���ԑ䐔�E���ԃf�[�^	�iID236�j
	uchar	area_num[2];		// ���䐔�E���ԃf�[�^	�iID237�j
	uchar	money_manage[2];	// ���K�Ǘ��f�[�^		�iID126�j(176 + 7 + 4) * 32
	uchar	turi_manage[2];		// �ޑK�Ǘ��W�v�f�[�^	�iID135�j
	uchar	gtotal[2];			// GT���v�W�v�f�[�^		�iID42,43,45,46,49,53�j
	uchar	rmon[2];			// ���u�Ď��f�[�^		�iID125�j
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	uchar	dummy2[22];
	uchar	long_park[2];		// �������ԏ�ԃf�[�^	�iID61�j
	uchar	dummy2[20];
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
} RAU_TABLE_DATA_COUNT;

/////////////////////////////////////////////////////////////////
//			�j�A�t���f�[�^�\����
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking;			// ���Ƀf�[�^			�iID20�j
	uchar	out_parking;		// �o�Ƀf�[�^			�iID21�j
	uchar	paid_data22;		// ���Z�f�[�^			�iID22�j
	uchar	paid_data23;		// ���Z�f�[�^			�iID23�j
	uchar	total;				// T���v�W�v�f�[�^		�iID30�`38�E41�j
	uchar	error;				// �G���[�f�[�^			�iID120�j
	uchar	alarm;				// �A���[���f�[�^		�iID121�j
	uchar	monitor;			// ���j�^�[�f�[�^		�iID122�j
	uchar	ope_monitor;		// ���샂�j�^�[�f�[�^	�iID123�j
	uchar	coin_total;			// �R�C�����ɏW�v�f�[�^	�iID131�j
	uchar	money_toral;		// �������ɏW�v�f�[�^	�iID133�j
	uchar	parking_num;		// ���ԑ䐔�E���ԃf�[�^	�iID236�j
	uchar	area_num;			// ���䐔�E���ԃf�[�^	�iID237�j
	uchar	turi_manage;		// �ޑK�Ǘ��W�v�f�[�^	�iID135�j
	uchar	money_manage;		// ���K�Ǘ��f�[�^		�iID126�j
	uchar	rmon;				// ���u�Ď��f�[�^		�iID125�j
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	uchar	dummy2[5];
	uchar	long_park;			// �������ԏ�ԃf�[�^	�iID61�j
	uchar	dummy2[4];
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
} RAU_NEAR_FULL;

/////////////////////////////////////////////////////////////////
//			�ʐM�v���f�[�^
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking;			// ���Ƀf�[�^			�iID20�j
	uchar	out_parking;		// �o�Ƀf�[�^			�iID21�j
	uchar	paid_data;			// ���Z�f�[�^			�iID22 23�j
	uchar	total;				// T���v�W�v�f�[�^		�iID30�`38�E41�j
	uchar	error;				// �G���[�f�[�^			�iID120�j
	uchar	alarm;				// �A���[���f�[�^		�iID121�j
	uchar	monitor;			// ���j�^�[�f�[�^		�iID122�j
	uchar	ope_monitor;		// ���샂�j�^�[�f�[�^	�iID123�j
	uchar	coin_total;			// �R�C�����ɏW�v�f�[�^	�iID131�j
	uchar	money_toral;		// �������ɏW�v�f�[�^	�iID133�j
	uchar	parking_num;		// ���ԑ䐔�E���ԃf�[�^	�iID236�j
	uchar	area_num;			// ���䐔�E���ԃf�[�^	�iID237�j
	uchar	money_manage;		// ���K�Ǘ��f�[�^		�iID126�j
	uchar	turi_manage;		// �ޑK�Ǘ��W�v�f�[�^	�iID135�j
	uchar	rmon;				// ���u�Ď��f�[�^		�iID125�j
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	uchar	dummy2[5];
	uchar	long_park;			// �������ԏ�ԃf�[�^	�iID61�j
	uchar	dummy2[4];
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
} RAU_SEND_REQUEST;

/////////////////////////////////////////////////////////////////
//			�e�[�u���N���A�v���f�[�^
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	in_parking;			// ���Ƀf�[�^			�iID20�j
	uchar	out_parking;		// �o�Ƀf�[�^			�iID21�j
	uchar	paid_data;			// ���Z�f�[�^			�iID22 23�j
	uchar	total;				// T���v�W�v�f�[�^		�iID30�`38�E41�j
	uchar	error;				// �G���[�f�[�^			�iID120�j
	uchar	alarm;				// �A���[���f�[�^		�iID121�j
	uchar	monitor;			// ���j�^�[�f�[�^		�iID122�j
	uchar	ope_monitor;		// ���샂�j�^�[�f�[�^	�iID123�j
	uchar	coin_total;			// �R�C�����ɏW�v�f�[�^	�iID131�j
	uchar	money_toral;		// �������ɏW�v�f�[�^	�iID133�j
	uchar	parking_num;		// ���ԑ䐔�E���ԃf�[�^	�iID236�j
	uchar	area_num;			// ���䐔�E���ԃf�[�^	�iID237�j
	uchar	money_manage;		// ���K�Ǘ��f�[�^		�iID126�j
	uchar	turi_manage;		// �ޑK�Ǘ��W�v�f�[�^	�iID135�j
	uchar	rmon;				// ���u�Ď��f�[�^		�iID125�j
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	uchar	dummy2[5];
	uchar	long_park;			// �������ԏ�ԃf�[�^	�iID61�j
	uchar	dummy2[4];
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
} RAU_CLEAR_REQUEST;

/////////////////////////////////////////////////////////////////
//			IBK����f�[�^
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
	uchar	table_data_count;			// �e�[�u�������f�[�^�v��
	uchar	rau_config1;				// ���uIBK�ݒ�f�[�^�P�v��
	uchar	rau_config2;				// ���uIBK�ݒ�f�[�^�Q�v��
	uchar	dummy2[15];
} RAU_CONTROL_REQUEST;

/////////////////////////////////////////////////////////////////
//			�ʐM�`�F�b�N[�v��/����]�f�[�^
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	NTpacket_len[2];		// �m�s�|�m�d�s�d���T�C�Y�{�Q ���o�b�t�@�ۑ����݂̂̍���
	uchar	data_len[2];			// ���ʕ��{�ŗL����(�O�J�b�g��)�f�[�^��
	uchar	data_len_zero_cut[2];	// �ŗL���O�J�b�g�O�̃f�[�^��(�T�O)
	uchar	dummy;
	uchar	id1;					// �h�c�P
	uchar	id2;					// �h�c�Q
	uchar	id3;					// �h�c�R
	uchar	id4;					// �h�c�S
	uchar	buffering_flag;			// �f�[�^�ێ��t���O       �����ʕ�
// ------------------------------------------------------------
	uchar	SeqNo;					// �V�[�P���V������       ���ŗL��
	uchar	Parking[4];				// ���ԏꇂ
	uchar	Model[2];				// �@��R�[�h
	uchar	Destination[4];			// ���M�懂
	uchar	Serial[6];				// �[���V���A����
	uchar	OpeTimeDate[6];			// �����N���������b
	uchar	CheckNo;				// �ʐM�`�F�b�N��
	union {
		struct {
			uchar	Kind;			// �v�����
			uchar	TimeOut;		// �^�C���A�E�g
			uchar	dummy[24];
		} quest;
		struct {
			uchar	Code;			// ���ʃR�[�h
			uchar	dummy[25];
		} sult;
	} Re;
	uchar	crc[2];					// �b�q�b ���o�b�t�@�ۑ����݂̂̍���
} RAU_COMMUNICATION_TEST;

/////////////////////////////////////////////////////////////////
//			Dopa�pNTNET�w�b�_�[
/////////////////////////////////////////////////////////////////
typedef struct {
	uchar	data_len[2];
	uchar	data_len_zero_cut[2];
	uchar	dummy;
	uchar	id1;
	uchar	id2;
	uchar	id3;
	uchar	id4;
	uchar	buffering_flag;
} RAU_NTDATA_HEADER;

/////////////////////////////////////////////////////////////////
//			NT-NET�p�@���u�ݒ�f�[�^�P
/////////////////////////////////////////////////////////////////
typedef struct {
	RAU_NTDATA_HEADER	nt_header;
	RAU_CONFIG			config_data1;
} RAU_CONFIG1;

/////////////////////////////////////////////////////////////////
//			NT-NET�p�@���u�ݒ�f�[�^�Q
/////////////////////////////////////////////////////////////////
typedef struct {
	RAU_NTDATA_HEADER	nt_header;
	DPA_RAU_CONFIG		config_data2;
} RAU_CONFIG2;

#endif	// RAUIDPROC_H
