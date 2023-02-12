#include "oled_utils.h"

#include <stdlib.h>
#include "oled_defs.h"
#include "hardware/i2c.h"

void oled_send_cmd(uint8_t cmd)
{
	// I2C write process expects a control byte followed by data
	// this "data" can be a command or data to follow up a command

	// Co = 1, D/C = 0 => the driver expects a command
	uint8_t buf[2] = {OLED_CONTROL_COMMAND, cmd};
	i2c_write_blocking(i2c_default, (OLED_ADDR & OLED_WRITE_MODE), buf, 2, false);
}

void oled_init()
{
	// some of these commands are not strictly necessary as the reset
	// process defaults to some of these but they are shown here
	// to demonstrate what the initialization sequence looks like

	// some configuration values are recommended by the board manufacturer

	oled_send_cmd(OLED_SET_DISP | 0x00); // set display off

	/* memory mapping */
	oled_send_cmd(OLED_SET_MEM_ADDR); // set memory address mode
	oled_send_cmd(0x00); // horizontal addressing mode

	/* resolution and layout */
	oled_send_cmd(OLED_SET_DISP_START_LINE); // set display start line to 0

	oled_send_cmd(OLED_SET_SEG_REMAP | 0x01); // set segment re-map
	// column address 127 is mapped to SEG0

	oled_send_cmd(OLED_SET_MUX_RATIO); // set multiplex ratio
	oled_send_cmd(OLED_HEIGHT - 1); // our display is only 32 pixels high

	oled_send_cmd(OLED_SET_COM_OUT_DIR | 0x08); // set COM (common) output scan direction
	// scan from bottom up, COM[N-1] to COM0

	oled_send_cmd(OLED_SET_DISP_OFFSET); // set display offset
	oled_send_cmd(0x00); // no offset

	oled_send_cmd(OLED_SET_COM_PIN_CFG); // set COM (common) pins hardware configuration
	oled_send_cmd(0x02); // manufacturer magic number

	/* timing and driving scheme */
	oled_send_cmd(OLED_SET_DISP_CLK_DIV); // set display clock divide ratio
	oled_send_cmd(0x80); // div ratio of 1, standard freq

	oled_send_cmd(OLED_SET_PRECHARGE); // set pre-charge period
	oled_send_cmd(0xF1); // Vcc internally generated on our board

	oled_send_cmd(OLED_SET_VCOM_DESEL); // set VCOMH deselect level
	oled_send_cmd(0x30); // 0.83xVcc

	/* display */
	oled_send_cmd(OLED_SET_CONTRAST); // set contrast control
	oled_send_cmd(0xFF);

	oled_send_cmd(OLED_SET_ENTIRE_ON); // set entire display on to follow RAM content

	oled_send_cmd(OLED_SET_NORM_INV); // set normal (not inverted) display

	oled_send_cmd(OLED_SET_CHARGE_PUMP); // set charge pump
	oled_send_cmd(0x14); // Vcc internally generated on our board

	oled_send_cmd(OLED_SET_SCROLL | 0x00); // deactivate horizontal scrolling if set
	// this is necessary as memory writes will corrupt if scrolling was enabled

	oled_send_cmd(OLED_SET_DISP | 0x01); // turn display on
}

void oled_alloc_data_buf(OLED_DATA_CMD* psDataCmd, uint16_t ui16DataSizeInBytes)
{
	// add another byte to the requested size to include the data message
	psDataCmd->pui8BasePtr = malloc(ui16DataSizeInBytes + 1);
	psDataCmd->pui8BasePtr[0] = OLED_CONTROL_DATA;
	psDataCmd->pui8DataPtr = &psDataCmd->pui8BasePtr[1];
	psDataCmd->ui16DataSizeInBytes = ui16DataSizeInBytes;
	// TODO: error handling
}

void oled_destroy_data_buf(OLED_DATA_CMD* psDataCmd)
{
	free(psDataCmd->pui8BasePtr);
}

void oled_send_data(OLED_DATA_CMD* psDataCmd)
{
	i2c_write_blocking(
		i2c_default,
		(OLED_ADDR & OLED_WRITE_MODE),
		psDataCmd->pui8BasePtr,
		psDataCmd->ui16DataSizeInBytes + 1,
		false
	);
}
