/*
 * ESP32 SPI bus implementation.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <string.h>
#include <lwiot.h>

#include <driver/spi_master.h>
#include <driver/spi_common.h>

#include <lwiot/log.h>
#include <lwiot/stl/string.h>
#include <lwiot/stl/move.h>

#include <lwiot/esp32/spibus.h>

namespace lwiot
{
	namespace esp32
	{
		SpiBus::SpiBus(int mosi, int miso, int clk, uint32_t freq) : lwiot::SpiBus(mosi, miso, clk, freq)
		{
			this->_config.miso_io_num = miso;
			this->_config.mosi_io_num = mosi;
			this->_config.sclk_io_num = clk;
			this->_config.quadhd_io_num = -1;
			this->_config.quadwp_io_num = -1;
			this->_config.max_transfer_sz = 8192;

			spi_bus_initialize(HSPI_HOST, &this->_config, SPI_DMA_CHANNEL);
		}

		bool SpiBus::transfer(lwiot::SpiMessage &msg)
		{
			esp_err_t ret;
			spi_device_handle_t spi;
			spi_transaction_t transaction;

			spi = this->setup(msg);
			memset(&transaction, 0, sizeof(transaction));
			transaction.length = msg.size();
			transaction.rx_buffer = msg.rxdata().data();
			transaction.tx_buffer = msg.txdata().data();

			ret = spi_device_transmit(spi, &transaction);
			this->stop(spi);

			return ret == ESP_OK;
		}

		spi_device_handle_t SpiBus::setup(const lwiot::SpiMessage &msg)
		{
			spi_device_handle_t spi;
			esp_err_t ret;
			spi_device_interface_config_t config;

			config.mode = SPI_MODE;
			config.clock_speed_hz = static_cast<int>(this->frequency());
			config.spics_io_num = msg.cspin();
			config.queue_size = SPI_QUEUE_SIZE;

			ret = spi_bus_add_device(HSPI_HOST, &config, &spi);

			if(ret != ESP_OK)
				return nullptr;

			return spi;
		}

		void SpiBus::stop(spi_device_handle_t spi)
		{
			spi_bus_remove_device(spi);
		}

		bool SpiBus::transfer(lwiot::stl::Vector<lwiot::SpiMessage> &msgs)
		{
			esp_err_t err = ESP_ERR_INVALID_STATE;
			spi_transaction_t transaction;

			if(msgs.size() <= 0)
				return false;

			auto device = this->setup(msgs[0]);

			for(auto& msg: msgs) {
				memset(&transaction, 0, sizeof(transaction));

				transaction.length = msg.size();
				transaction.rx_buffer = msg.rxdata().data();
				transaction.tx_buffer = msg.txdata().data();

				err = spi_device_transmit(device, &transaction);

				if(err != ESP_OK)
					break;
			}

			this->stop(device);
			return err == ESP_OK;
		}
	}
}
