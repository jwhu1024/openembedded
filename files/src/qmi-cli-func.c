#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "qmi-cli-func.h"
#include "utils.h"

static nv_item_type nv_item_data;
static qmi_cli_status_t rc = QMI_CLI_FAIL;

// pointer to qmi read/write function
static int (*qmi_func_ptr)(int nvid, uint8_t DataBuffer[], int buffersize);

// prototype declare
static qmi_cli_status_t nv_prl_enable_op	(qmi_req *info);
static qmi_cli_status_t nv_ftm_mode_op		(qmi_req *info);
static qmi_cli_status_t nv_ue_imei_op		(qmi_req *info);
static qmi_cli_status_t nv_br_project_op	(qmi_req *info);
static qmi_cli_status_t nv_pref_mode_op		(qmi_req *info);
static qmi_cli_status_t nv_test_op		(qmi_req *info);

/*
	Whole functions in this table are communicate with QMI,
	and proceeds NV read/write actions.
*/
qmi_cli_op_table_t qmi_cli_op_func_tbl = {
	nv_prl_enable_op,	/* 256-NV_PRL_ENABLED_I		*/
	nv_ftm_mode_op,		/* 453-NV_FTM_MODE_I		*/
	nv_ue_imei_op, 		/* 550-NV_UE_IMEI_I		*/
	nv_br_project_op,	/* 5079-NV_UNDP_HSU_PRODSTR_I	*/
	nv_pref_mode_op,	/* 10-NV_PREF_MODE_I		*/
	nv_test_op
};

/*
	Function implement by NV item,
	since every item have own structure and types.
*/
static qmi_cli_status_t nv_test_op (qmi_req *info) {
	/* This function just used to test only */
	QMI_NV_ReleaseCmdPager();
	return rc;
}

static qmi_cli_status_t nv_pref_mode_op (qmi_req *info) {
	rc = QMI_CLI_FAIL;
	switch (info->op_code) {
		case QMI_CLI_READ:
			qmi_func_ptr = &send_qmi_nv_read;
			break;
		case QMI_CLI_WRITE:
			nv_item_data.pref_mode.mode = (nv_mode_enum_type) atoi ((char *)info->nvdata);
			qmi_func_ptr = &send_qmi_nv_write;
			break;
	}

	rc = (*qmi_func_ptr)(info->nv_item,			/* NV_PREF_MODE_I */
			     (uint8_t *)&(nv_item_data.pref_mode),
			     sizeof(nv_item_data.pref_mode));

	_dbg("nv_pref_mode_op - %s - %d\n",
		(rc == QMI_CLI_SUCCESS) ? "SUCCESS" : "FAILED",
		(rc == QMI_CLI_SUCCESS) ? nv_item_data.pref_mode.mode : -1);

	QMI_NV_ReleaseCmdPager();
	return rc;
}

static qmi_cli_status_t nv_prl_enable_op (qmi_req *info) {
	nv_enabled_type enabled_type;
	memset(&enabled_type, 0, sizeof(enabled_type));

	rc = QMI_CLI_FAIL;

	switch (info->op_code) {
		case QMI_CLI_READ:
			qmi_func_ptr = &send_qmi_nv_read;
			break;
		case QMI_CLI_WRITE:
			enabled_type.enabled = info->nvdata[0];
			qmi_func_ptr = &send_qmi_nv_write;
			break;
	}

	rc = (*qmi_func_ptr)(info->nv_item,			/* NV_PRL_ENABLED_I */
			     (uint8_t *)&(enabled_type),
			     sizeof(enabled_type));

	_dbg("nv_prl_enable_op - %s - %c\n",
		(rc == QMI_CLI_SUCCESS) ? "SUCCESS" : "FAILED",
		(rc == QMI_CLI_SUCCESS) ? enabled_type.enabled : '\0');

	QMI_NV_ReleaseCmdPager();
	return rc;
}

static qmi_cli_status_t nv_ftm_mode_op (qmi_req *info) {
	rc = QMI_CLI_FAIL;
	uint8_t ftm_mode = (uint8_t) atoi ((char *)info->nvdata);

	switch (info->op_code) {
		case QMI_CLI_READ:
			qmi_func_ptr = &send_qmi_nv_read;
			break;
		case QMI_CLI_WRITE:
			qmi_func_ptr = &send_qmi_nv_write;
			break;
	}

	rc = (*qmi_func_ptr)(info->nv_item,			/* NV_FTM_MODE_I */
			     (uint8_t *)&ftm_mode,
			     sizeof(uint8_t));
	
	_dbg("nv_ftm_mode_op - %s - %d\n",
		(rc == QMI_CLI_SUCCESS) ? "SUCCESS" : "FAILED",
		(rc == QMI_CLI_SUCCESS) ? ftm_mode : -1);

	QMI_NV_ReleaseCmdPager();
	return rc;
}

static qmi_cli_status_t nv_ue_imei_op ( qmi_req *info ) {
	uint8_t irc = -1;
	uint8_t imei_hex[NV_UE_IMEI_SIZE] = {0};
	uint8_t imei_ascii[HSU_CFG_SELECTOR_MAX_ESN_IMEI_SIZE+1];

	switch (info->op_code) {
		case QMI_CLI_READ:
			rc = (qmi_cli_status_t) BR_read_imei (imei_ascii);
			_dbg("nv_ue_imei_op - read %s - %s\n",
					(rc == QMI_CLI_SUCCESS) ? "SUCCESS" : "FAILED",
					imei_ascii);
			break;
		case QMI_CLI_WRITE:
			// convert imei from ascii to hex 004400152020000
			irc = imei_ascii_to_hex(info->nvdata, imei_hex);
			_dbg("Convert IMEI from ascii to hex %s\n",
					(irc == 0) ? "SUCCESS" : "FAILED");

			if (irc == 0) {
				// write IMEI 550
				rc = send_qmi_efs_put("/nvm/num/550", imei_hex, sizeof(imei_hex));
				_dbg("nv_ue_imei_op - write %s - %s\n",
					(rc == QMI_CLI_SUCCESS) ? "SUCCESS" : "FAILED",
					info->nvdata);
			} else {
				rc = QMI_CLI_FAIL;
			}
			break;
		case QMI_CLI_DELETE:
			rc = (qmi_cli_status_t) send_qmi_efs_remove("/nvm/num/550");
			_dbg("nv_ue_imei_op - delete %s\n",
				(rc == QMI_CLI_SUCCESS) ? "SUCCESS" : "FAILED");
			break;
	}

	QMI_NV_ReleaseCmdPager();
	return rc;
}

static qmi_cli_status_t nv_br_project_op (qmi_req *info) {
	memset(nv_item_data.undp_hsu_prodstr, 0, sizeof(nv_item_data.undp_hsu_prodstr));

	switch (info->op_code) {
		case QMI_CLI_READ:
			qmi_func_ptr = &send_qmi_nv_read;
			break;
		case QMI_CLI_WRITE:
			// TBD
			break;
	}

	rc = (*qmi_func_ptr)(info->nv_item,			/* NV_UNDP_HSU_PRODSTR_I */
			     (uint8_t *)&nv_item_data.undp_hsu_prodstr,
			     sizeof(nv_item_data.undp_hsu_prodstr));

	_dbg("nv_br_project_op - %s - %s\n",
		(rc == QMI_CLI_SUCCESS) ? "SUCCESS" : "FAILED",
		(rc == QMI_CLI_SUCCESS) ? nv_item_data.undp_hsu_prodstr : NULL);
	
	QMI_NV_ReleaseCmdPager();
	return rc;
}
