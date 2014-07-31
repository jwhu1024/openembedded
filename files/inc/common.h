#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <string.h>

#define NV_DATA_MAXSIZE	256

#define __SHORT_FILE__ ((strrchr(__FILE__, '/'))?  strrchr(__FILE__, '/') + 1 : __FILE__)
#ifdef __DEBUG__
	#define _dbg(message, ...) printf("[LOG][%s:%d] " message, __SHORT_FILE__, __LINE__, ##__VA_ARGS__)
#else
	#define _dbg(message, ...)
#endif

typedef enum {
	QMI_CLI_FAIL = 0,
	QMI_CLI_SUCCESS,
	QMI_CLI_RANGE_FAIL,
	QMI_CLI_ITEM_NOT_SUPPORT
} qmi_cli_status_t;

typedef enum {
	QMI_CLI_READ	= 1,
	QMI_CLI_WRITE	= 2,
	QMI_CLI_DELETE	= 3
} qmi_cli_opcode_t;

typedef struct qmi_req {
	uint8_t op_code;
	uint16_t nv_item;
	uint8_t nvdata[NV_DATA_MAXSIZE];
} qmi_req;

#endif /* _COMMON_H_ */
