#ifndef _QMI_CLI_FUNC_
#define _QMI_CLI_FUNC_

#include "common.h"
#include "qmi_nv_lib.h"

typedef struct qmi_cli_op_table {
	qmi_cli_status_t (*nv_prl_enable_op)	(qmi_req *info);
	qmi_cli_status_t (*nv_ftm_mode_op)	(qmi_req *info);
	qmi_cli_status_t (*nv_ue_imei_op)	(qmi_req *info);
	qmi_cli_status_t (*nv_br_project_op)	(qmi_req *info);
	qmi_cli_status_t (*nv_pref_mode_op)	(qmi_req *info);
	qmi_cli_status_t (*nv_test_op)		(qmi_req *info);
} qmi_cli_op_table_t;

#endif /* _QMI_CLI_FUNC_ */