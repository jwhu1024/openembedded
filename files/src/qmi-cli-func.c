#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "qmi-cli-func.h"

static qmi_cli_status_t nv_ftm_mode_op (qmi_req *info);

/*
	Whole functions in this table are communicate with QMI,
	and proceeds NV read/write actions.
*/
qmi_cli_op_table_t qmi_cli_op_func_tbl = {
	nv_ftm_mode_op 		// NV_FTM_MODE_I - 453
};

/*
	Function implement by NV item,
	since every item have own structure and types.
*/
static qmi_cli_status_t nv_ftm_mode_op (qmi_req *info) {
	_dbg("nv_ftm_mode_op\n");
	_dbg("info->nv_item : %d\n", info->nv_item);
	_dbg("info->op_code : %d\n", info->op_code);
	
	uint8_t ftm_mode = 0;
	qmi_cli_opcode_t rc = QMI_CLI_FAIL;
	switch (info->op_code) {
		case QMI_CLI_READ:
			if ( rc = send_qmi_nv_read( NV_FTM_MODE_I, (uint8_t *)&ftm_mode, sizeof(uint8_t) ) == QMI_CLI_SUCCESS ) {
				_dbg("NV_FTM_MODE_I Read Succeed - %d\n", ftm_mode);
			}
		break;

		case QMI_CLI_WRITE:
			ftm_mode = (uint8_t) atoi ((char *)info->nvdata);
			if ( rc = send_qmi_nv_write( NV_FTM_MODE_I, (uint8_t *)&ftm_mode, sizeof(uint8_t) ) == QMI_CLI_SUCCESS ) {
				_dbg("NV_FTM_MODE_I Write Succeed!\n");
			}
			_dbg("ftm_mode is : %d\n", ftm_mode);
		break;
	}

	_dbg("rc = %d\n", (int) rc);
	QMI_NV_ReleaseCmdPager();
	return rc;
}