#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "incbin.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "oled_defs.h"
#include "oled_utils.h"

// Based on:
// https://github.com/raspberrypi/pico-examples/tree/master/i2c/ssd1306_i2c

INCBIN(Image, "../../turtle/turtle.bin");
// INCBIN(Image, "../../twinpeaks/trimmed/twinpeaks.bin");

int main()
{
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
	#error "i2c pins not defined"
#endif

	// useful information for picotool
	bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
	bi_decl(bi_program_description("updating the whole oled at once"));

	// stdio_init_all();

	// I2C is "open drain", pull ups to keep signal high when no data is being
	// sent
	i2c_init(i2c_default, 400 * 1000);
	gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
	gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
	gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

	// run through the complete initialization process
	oled_init();

	{
		// Set the cursor start position and draw area to the full display
		// if we always send a full screen's worth of data, this should stay the
		// same
		oled_send_cmd(OLED_SET_COL_ADDR);
		oled_send_cmd(0); // start column 0
		oled_send_cmd(OLED_WIDTH - 1); // end column

		oled_send_cmd(OLED_SET_PAGE_ADDR);
		oled_send_cmd(0); // start page 0
		oled_send_cmd(OLED_NUM_PAGES - 1); // end page

		// Set up the image command
		OLED_DATA_CMD sImageCmd;
		oled_alloc_data_buf(&sImageCmd, OLED_BUF_LEN);
		// fill with black pixels
		memset(sImageCmd.pui8DataPtr, 0x00, OLED_BUF_LEN);
		oled_send_data(&sImageCmd);

		// intro sequence: flash the screen 3 times
		for (int i = 0; i < 3; i++) {
			oled_send_cmd(0xA5); // ignore RAM, all pixels on
			sleep_ms(500);
			oled_send_cmd(0xA4); // go back to following RAM
			sleep_ms(500);
		}

		// get framecount from the embed
		uint32_t ui32FrameCount;
		memcpy(&ui32FrameCount, (void*)gImageData, sizeof(uint32_t));
		const uint8_t* pui8ImageData = gImageData + sizeof(uint32_t);

		if (ui32FrameCount > 1)
		{
			uint32_t i = 0;
			while(1)
			{
				memcpy(
					sImageCmd.pui8DataPtr,
					pui8ImageData + (i++ * OLED_BUF_LEN),
					OLED_BUF_LEN
				);

				oled_send_data(&sImageCmd);
				sleep_ms(41);

				i = i % ui32FrameCount;
			}
		}
		else
		{
			memcpy(sImageCmd.pui8DataPtr, pui8ImageData, OLED_BUF_LEN);
			oled_send_data(&sImageCmd);
		}
	}

	return 0;
}
