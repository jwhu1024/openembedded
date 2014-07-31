#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <stdbool.h>
#include "qmi-cli.h"
#include "qmi-cli-handler.h"

#define PROGRAM_NAME	"qmi-cli"
#define PROGRAM_VER		"1.0.2"

static qmi_req _qmi_req = {
	.op_code = -1,
	.nv_item = -1
};

/* helper functions */
void display_version(void) {
	printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VER);
	exit(EXIT_SUCCESS);
}

void display_help(int status) {
	printf("%s | version %s | %s | %s\n", PROGRAM_NAME,
										PROGRAM_VER,
										__TIME__,
										__DATE__);

	fprintf(status == EXIT_SUCCESS ? stdout : stderr,
	"Usage: qmi-cli -i [NVITEM] -a [ACTION|]			\n"
	"Info : Read/Write NV item through QMI				\n\n"
	"  -i, specify the NV item							\n"
	"  -a, Action you wants 1:Read | 2:Write | 3:Delete	\n"
	"  -p, payload if op code is 2(write)				\n"
	"  -h, Display this help and exit					\n"
	"  -v, Output version information and exit			\n"
	);
	exit(status);
}

/* entry */
int main(int argc, char *argv[]) {
	int option = 0;
	bool err = false;
	qmi_req *qr = &_qmi_req;

	memset(qr->nvdata, '\0', sizeof(qr->nvdata));

	// parse command line arguments
	while ((option = getopt(argc, argv,"vhi:a:p:")) != -1) {
		switch (option) {
		case 'i' :
			qr->nv_item = atoi(optarg);
			_dbg("-i %d\r\n", qr->nv_item);
			break;
		case 'a' :
			qr->op_code = atoi(optarg);
			_dbg("-a %d\r\n", qr->op_code);
			break;
		case 'p' :
			snprintf((char *)qr->nvdata, sizeof(qr->nvdata), "%s", optarg);
			_dbg("-p %s\r\n", qr->nvdata);
			break;
		case 'v' :
			display_version();
			break;
		case 'h' :
			display_help(EXIT_SUCCESS);
			break;
		}
	}

	_dbg("op_code : %d, nv_item : %d, nvdata : %s\n", qr->op_code, qr->nv_item, qr->nvdata);

	// error conditions
	err |= (qr->nv_item == -1);
	err |= (qr->op_code == -1);
	err |= (qr->op_code == 2 && qr->nvdata[0] == '\0');

	if (err) {
		display_help(EXIT_FAILURE);
	}

#ifdef __DEBUG__
	_dbg("Function works? %s\n", ((int) handler_main(qr) == 1) ? "SUCCESS" : "FAILED");
	return 1;
#else
	return (int) handler_main(qr);
#endif

}