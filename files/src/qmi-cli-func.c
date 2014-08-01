#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "qmi-cli-func.h"

static int BR_read_imei (char *return_buff_ptr);

// pointer to qmi read/write function
static int (*qmi_func_ptr)(int nvid, uint8_t DataBuffer[], int buffersize);

// prototype declare
static qmi_cli_status_t nv_prl_enable_op (qmi_req *info);
static qmi_cli_status_t nv_ftm_mode_op (qmi_req *info);
static qmi_cli_status_t nv_ue_imei_op (qmi_req *info);

/*
	Whole functions in this table are communicate with QMI,
	and proceeds NV read/write actions.
*/
qmi_cli_op_table_t qmi_cli_op_func_tbl = {
	nv_prl_enable_op,	/* 256-NV_PRL_ENABLED_I */
	nv_ftm_mode_op,		/* 453-NV_FTM_MODE_I 	*/
	nv_ue_imei_op 		/* 550-NV_UE_IMEI_I 	*/
};

/*
	Function implement by NV item,
	since every item have own structure and types.
*/
static qmi_cli_status_t nv_prl_enable_op (qmi_req *info) {
	_dbg("info->nv_item : %d\n", info->nv_item);
	_dbg("info->op_code : %d\n", info->op_code);

	nv_enabled_type enabled_type;
	memset(&enabled_type, 0, sizeof(enabled_type));

	qmi_cli_opcode_t rc = QMI_CLI_FAIL;

	switch (info->op_code) {
		case QMI_CLI_READ:
			qmi_func_ptr = &send_qmi_nv_read;
			break;
		case QMI_CLI_WRITE:
			enabled_type.nam = 0;			
			enabled_type.enabled = info->nvdata[0];
			qmi_func_ptr = &send_qmi_nv_write;
			break;
	}

	// nv_item_type brtype;
	// rc = (*qmi_func_ptr)(NV_PRL_ENABLED_I, (uint8_t_t *)&(brtype), sizeof(brtype));
	// _dbg("enabled : %c, nam : %d, rc = %d\n", brtype.prl_enabled.enabled, brtype.prl_enabled.nam,(int) rc);
	rc = (*qmi_func_ptr)(NV_PRL_ENABLED_I, (uint8_t *)&(enabled_type), sizeof(enabled_type));
	_dbg("enabled : %c, nam : %d, rc = %d\n", enabled_type.enabled, enabled_type.nam,(int) rc);
	QMI_NV_ReleaseCmdPager();
	return rc;
}

static qmi_cli_status_t nv_ftm_mode_op (qmi_req *info) {
	_dbg("info->nv_item : %d\n", info->nv_item);
	_dbg("info->op_code : %d\n", info->op_code);
	
	qmi_cli_opcode_t rc = QMI_CLI_FAIL;
	uint8_t ftm_mode = (uint8_t) atoi ((char *)info->nvdata);
	
	switch (info->op_code) {
		case QMI_CLI_READ:
			qmi_func_ptr = &send_qmi_nv_read;
			break;
		case QMI_CLI_WRITE:
			qmi_func_ptr = &send_qmi_nv_write;
			break;
	}

	rc = (*qmi_func_ptr)(NV_FTM_MODE_I, (uint8_t *)&ftm_mode, sizeof(uint8_t));
	
	_dbg("ftm_mode : %d, rc = %d\n", ftm_mode, (int) rc);
	QMI_NV_ReleaseCmdPager();
	return rc;
}

static qmi_cli_status_t nv_ue_imei_op ( qmi_req *info ) {
	_dbg("info->nv_item : %d\n", info->nv_item);
	_dbg("info->op_code : %d\n", info->op_code);

	char imei_ascii[HSU_CFG_SELECTOR_MAX_ESN_IMEI_SIZE+1];
	if (BR_read_imei(imei_ascii)) {
		_dbg("IMEI : %s\n", imei_ascii);
	}

	return QMI_CLI_SUCCESS;
}

/* helper functions */
static int BR_read_imei (char *return_buff_ptr) {
	uint8_t imei_bcd_len = 0;
	uint8_t n = 0;
	uint8_t digit;
	nv_item_type      nv_item_data;
	char imei_ascii[HSU_CFG_SELECTOR_MAX_ESN_IMEI_SIZE+1];

	if (!send_qmi_nv_read(NV_UE_IMEI_I, (uint8_t *)&(nv_item_data.ue_imei), sizeof(nv_item_data.ue_imei))) {
		return 0;
	}
	
	imei_bcd_len = nv_item_data.ue_imei.ue_imei[0];

	if (imei_bcd_len <= (NV_UE_IMEI_SIZE-1)) {
		/* This is a valid IMEI */
		memset(imei_ascii, 0, HSU_CFG_SELECTOR_MAX_ESN_IMEI_SIZE+1);

		for (n = 1; n <= imei_bcd_len; n++) {
			digit = nv_item_data.ue_imei.ue_imei[n] & 0x0F;
			if ((digit <= 9) || (n <= 1)) {
				imei_ascii[ (n - 1) * 2 ] = digit + '0';
			} else {
				imei_ascii[ (n - 1) * 2 ] = '\0';
				break;
			}

			digit = nv_item_data.ue_imei.ue_imei[n] >> 4;
			if ((digit <= 9) || (n <= 1)) {
				imei_ascii[ ((n - 1) * 2) + 1 ] = digit + '0';
			} else {
				imei_ascii[ ((n - 1) * 2) + 1 ] = '\0';
				break;
			}
		}
		imei_ascii[HSU_CFG_SELECTOR_MAX_ESN_IMEI_SIZE] = '\0';
		sprintf( return_buff_ptr, "%s", imei_ascii + 1 );
	}
	return 1;
}
