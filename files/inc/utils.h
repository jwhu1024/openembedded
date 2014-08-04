#ifndef _UTILS_H_
#define _UTILS_H_

#define HSU_CFG_SELECTOR_MAX_ESN_IMEI_SIZE (NV_UE_IMEI_SIZE-1) * 2

int BR_read_imei (uint8_t return_buff_ptr[]);
uint8_t imei_hex_to_ascii(uint8_t buf[], uint8_t imei_ascii[]);
uint8_t imei_ascii_to_hex(uint8_t imei_ascii[], uint8_t buf[]);

#endif /* _UTILS_H_ */