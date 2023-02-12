#ifndef _OLED_UTILS_H_
#define _OLED_UTILS_H_

#include <stdint.h>

void oled_send_cmd(uint8_t cmd);

void oled_init();

typedef struct _OLED_DATA_CMD
{
	uint8_t* pui8BasePtr;
	uint8_t* pui8DataPtr;
	uint16_t ui16DataSizeInBytes;
} OLED_DATA_CMD;

void oled_alloc_data_buf(OLED_DATA_CMD* psDataCmd, uint16_t ui16DataSizeInBytes);

void oled_destroy_data_buf(OLED_DATA_CMD* psDataCmd);

void oled_send_data(OLED_DATA_CMD* psDataCmd);


#endif /* _OLED_UTILS_H_ */
