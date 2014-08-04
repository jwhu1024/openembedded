/*
	IMEI-related functions copied from
	~\br-3032\apps_proc\oe-core\meta-msm\recipes\customization\br-imeifix\imei-util.c
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"
#include "qmi_nv_lib.h"

int BR_read_imei (uint8_t return_buff_ptr[]) {
	uint8_t imei_bcd_len = 0;
	uint8_t n = 0;
	uint8_t digit;
	char imei_ascii[HSU_CFG_SELECTOR_MAX_ESN_IMEI_SIZE+1];
	nv_item_type nv_item_data;

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
		sprintf( (char*)return_buff_ptr, "%s", imei_ascii + 1 );
	}
	return 1;
}

uint8_t imei_ascii_to_hex(uint8_t imei_ascii[], uint8_t buf[]) {
	uint8_t i;
	char low_byte, high_byte;

	buf[0] = 0x08;
	high_byte = imei_ascii[0];

	if (! (('0' <= high_byte) && (high_byte <= '9'))) {
		// error handling
		return -1;
	}

	buf[1] = (((high_byte - '0') << 4) & 0xF0) | 0x0A;

	for (i=1; i<8; i++) {
		low_byte = imei_ascii[i+i-1];
		high_byte = imei_ascii[i+i];
		if (! (('0' <= low_byte) && (low_byte <= '9') && 
		   ('0' <= high_byte) && (high_byte <= '9'))) {
			// error handling
			return -1;
		}
		buf[i+1] = (((high_byte - '0') << 4) & 0xF0) | ((low_byte - '0') & 0x0F);
	}
	return 0;
}


uint8_t imei_hex_to_ascii(uint8_t buf[], uint8_t imei_ascii[])
{
	const char dec_to_char[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	uint8_t digit, i;

	digit = (buf[1] >> 4);
	if (! ((0<=digit) && (digit<=9))) {
		// error handling
		return -1;
	}

	imei_ascii[0] = dec_to_char[digit];

	for (i=1; i<8; i++) {
		digit = buf[i+1] & 0x0F;
		if (! ((0<=digit) && (digit<=9))) {
			// error handling
			return -1;
		}

		imei_ascii[i+i-1] = dec_to_char[digit];
		digit = (buf[i+1] >> 4) & 0x0F;
		if (! ((0<=digit) && (digit<=9))) {
			// error handling
			return -1;
		}
		imei_ascii[i+i] = dec_to_char[digit];
	}

	imei_ascii[15] = '\0';
	return 0;
}