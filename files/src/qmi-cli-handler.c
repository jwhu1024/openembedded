#include <stdio.h>
#include "qmi-cli-handler.h"
#include "qmi-cli-func.h"

extern qmi_cli_op_table_t qmi_cli_op_func_tbl;

/*
	function pointer prototype
*/
static qmi_cli_status_t (*func_ptr)(qmi_req *info);

/*
	Routes the request to corresponding handler
*/
qmi_cli_status_t handler_main (qmi_req *req_info) {
	_dbg("req_info->nv_item : %d\n", req_info->nv_item);
	_dbg("req_info->op_code : %d\n", req_info->op_code);
	_dbg("req_info->nvdata  : %s\n", req_info->nvdata);

	qmi_cli_status_t rc = QMI_CLI_FAIL;
	func_ptr = NULL;

	switch (req_info->nv_item) {
		case NV_FTM_MODE_I:
			func_ptr = qmi_cli_op_func_tbl.nv_ftm_mode_op;
			break;
		case NV_PRL_ENABLED_I:
			func_ptr = qmi_cli_op_func_tbl.nv_prl_enable_op;
			break;
		case NV_UE_IMEI_I:
			func_ptr = qmi_cli_op_func_tbl.nv_ue_imei_op;
			break;
		case NV_UNDP_HSU_PRODSTR_I:
			func_ptr = qmi_cli_op_func_tbl.nv_br_project_op;
			break;
		case 1024:
			func_ptr = qmi_cli_op_func_tbl.nv_test_op;
			break;
		default:
			rc = QMI_CLI_ITEM_NOT_SUPPORT;
			break;
	}

	if (func_ptr) {
		return (*func_ptr)(req_info);
	}

	return rc;
}