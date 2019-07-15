/*
 * Hardware I2C algorithm definition.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <lwiot.h>

#include <lwiot/types.h>
#include <lwiot/log.h>
#include <lwiot/sharedpointer.h>

#include <lwiot/io/i2calgorithm.h>
#include <lwiot/io/gpiopin.h>
#include <lwiot/kernel/lock.h>

#include <lwiot/esp32/hardwarei2calgorithm.h>

#include <driver/i2c.h>
#include <driver/gpio.h>
#include <driver/periph_ctrl.h>

static i2c_config_t config;
static volatile i2c_cmd_handle_t handle;
#define TIMEOUT 500

namespace lwiot
{
	namespace esp32
	{
		HardwareI2CAlgorithm::HardwareI2CAlgorithm(const lwiot::GpioPin &scl,
				const lwiot::GpioPin &sda, uint32_t frequency) :
				lwiot::HardwareI2CAlgorithm(scl, sda, frequency), _port(I2C_NUM_0)
		{
			config.mode = I2C_MODE_MASTER;
			config.sda_io_num = (gpio_num_t) sda.pin();
			config.scl_io_num = (gpio_num_t) scl.pin();
			config.scl_pullup_en = GPIO_PULLUP_DISABLE;
			config.sda_pullup_en = GPIO_PULLUP_DISABLE;
			config.master.clk_speed = frequency;

			ESP_ERROR_CHECK(i2c_param_config(this->_port, &config))
			ESP_ERROR_CHECK(i2c_driver_install(this->_port, config.mode, 0, 0, 0))
			ESP_ERROR_CHECK(i2c_set_timeout(this->_port, 20000))
			ESP_ERROR_CHECK(i2c_filter_enable(this->_port, 4))
			this->patchClock();

			handle = nullptr;
		}

		HardwareI2CAlgorithm::HardwareI2CAlgorithm(HardwareI2CAlgorithm &&other) noexcept :
			lwiot::HardwareI2CAlgorithm(stl::forward<HardwareI2CAlgorithm>(other)), _port(I2C_NUM_0)
		{
		}

		HardwareI2CAlgorithm::~HardwareI2CAlgorithm()
		{
			if(this->_port >= I2C_NUM_MAX)
				return;

			i2c_driver_delete(this->_port);
		}

		HardwareI2CAlgorithm& HardwareI2CAlgorithm::operator=(HardwareI2CAlgorithm &&rhs) noexcept
		{
			this->move(rhs);
			this->_port = rhs._port;

			rhs._port = I2C_NUM_MAX;
			return *this;
		}

		void HardwareI2CAlgorithm::patchClock() const
		{
#ifdef CONFIG_PATCH_I2C_CLOCK
			int hi, lo;

			i2c_get_period(this->_port, &hi, &lo);

			hi -= 7;
			lo -= 7;

			i2c_set_period(this->_port, hi, lo);
#endif
		}

		void HardwareI2CAlgorithm::start(uint16_t sla, bool repeated)
		{
			UNUSED(repeated);

			if(handle == nullptr)
				handle = i2c_cmd_link_create();

			i2c_master_start(handle);
			i2c_master_write_byte(handle, sla & 0xFF, true);
		}

		void HardwareI2CAlgorithm::stop()
		{
			i2c_master_stop(handle);
		}

		void HardwareI2CAlgorithm::write(const uint8_t *byte, bool ack)
		{
			UNUSED(ack);
			i2c_master_write_byte(handle, *byte, true);
		}

		void HardwareI2CAlgorithm::write(const uint8_t *bytes, size_t length, bool ack)
		{
			UNUSED(ack);
			i2c_master_write(handle, (uint8_t*)bytes, length, true);
		}

		void HardwareI2CAlgorithm::read(uint8_t *byte, bool ack)
		{

			i2c_ack_type_t should_ack = ack ? I2C_MASTER_ACK : I2C_MASTER_LAST_NACK;
			i2c_master_read_byte(handle, byte, should_ack);
		}

		void HardwareI2CAlgorithm::read(uint8_t *bytes, size_t length, bool ack)
		{
			i2c_ack_type_t should_ack = ack ? I2C_MASTER_ACK : I2C_MASTER_LAST_NACK;
			i2c_master_read(handle, bytes, length, should_ack);
		}

		int HardwareI2CAlgorithm::flush() const
		{

			esp_err_t err;

			err = i2c_master_cmd_begin(this->_port, handle, TIMEOUT / portTICK_PERIOD_MS);

			if(err != ESP_OK) {
				print_dbg("ESP32 I2C error: %i\n", err);
				return -EINVALID;
			}

			return -EOK;
		}

		void HardwareI2CAlgorithm::reset()
		{
			if(handle == nullptr)
				return;

			i2c_cmd_link_delete(handle);
			handle = nullptr;
		}

		void HardwareI2CAlgorithm::setFrequency(const uint32_t &freq)
		{
			lwiot::HardwareI2CAlgorithm::setFrequency(freq);

			config.master.clk_speed = freq;
			i2c_param_config(this->_port, &config);
		}
	}
}
