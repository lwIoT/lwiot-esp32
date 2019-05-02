/*
 * Implementation of the I2C HAl.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <stdint.h>
#include <lwiot.h>
#include <string.h>

#include <lwiot/error.h>
#include <lwiot/log.h>

#include <lwiot/io/i2chal.h>

#include <driver/i2c.h>
#include <driver/periph_ctrl.h>

static i2c_config_t config;
static volatile i2c_cmd_handle_t handle;

#define TIMEOUT 500

void i2c_set_frequency(uint32_t freq)
{
	config.master.clk_speed = freq;
	i2c_param_config(I2C_NUM_0, &config);
}

#ifdef CONFIG_PATCH_I2C_CLOCK
static void patch_i2c_clock()
{
	int hi, lo;

	i2c_get_period(I2C_NUM_0, &hi, &lo);

	hi -= 10;
	lo -= 10;

	i2c_set_period(I2C_NUM_0, hi, lo);
}
#else
static void patch_i2c_clock()
{
}
#endif

void i2c_setup(int scl, int sda, uint32_t freq)
{
	config.mode = I2C_MODE_MASTER;
	config.sda_io_num = (gpio_num_t)(sda);
	config.scl_io_num = (gpio_num_t)(scl);
	config.scl_pullup_en = GPIO_PULLUP_DISABLE;
	config.sda_pullup_en = GPIO_PULLUP_DISABLE;
	config.master.clk_speed = freq;

	i2c_param_config(I2C_NUM_0, &config);
	i2c_driver_install(I2C_NUM_0, config.mode, 0, 0, 0);
	i2c_set_timeout(I2C_NUM_0, 20000);
	patch_i2c_clock();

	handle = NULL;
}

void i2c_disable()
{
	i2c_driver_delete(I2C_NUM_0);
}

void i2c_reset()
{
	if(handle == NULL)
		return;

	i2c_cmd_link_delete(handle);
	handle = NULL;
}

void i2c_start(uint16_t sla, bool repeated)
{
	UNUSED(repeated);

	if(handle == NULL)
		handle = i2c_cmd_link_create();

	i2c_master_start(handle);
	i2c_master_write_byte(handle, sla & 0xFF, true);
}

void i2c_stop()
{
	i2c_master_stop(handle);
}

void i2c_write_byte(const uint8_t *byte, bool ack)
{
	UNUSED(ack);
	i2c_master_write_byte(handle, *byte, true);
}

void i2c_write_buffer(const uint8_t *bytes, size_t length, bool ack)
{
	i2c_master_write(handle, (uint8_t*)bytes, length, true);
}

void i2c_read_byte(uint8_t* byte, bool ack)
{
	i2c_ack_type_t should_ack = ack ? I2C_MASTER_ACK : I2C_MASTER_LAST_NACK;
	i2c_master_read_byte(handle, byte, should_ack);
}

void i2c_read_buffer(uint8_t* bytes, size_t length, bool ack)
{
	i2c_ack_type_t should_ack = ack ? I2C_MASTER_ACK : I2C_MASTER_LAST_NACK;
	i2c_master_read(handle, bytes, length, should_ack);
}

int i2c_write_buffers()
{
	esp_err_t err;

	err = i2c_master_cmd_begin(I2C_NUM_0, handle, TIMEOUT / portTICK_PERIOD_MS);

	if(err != ESP_OK) {
		print_dbg("ESP32 I2C error: %i\n", err);
		return -EINVALID;
	}

	return -EOK;
}
