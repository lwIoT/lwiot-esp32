/*
 * ESP32 SPI bus implementation.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#pragma once

#include <stdlib.h>
#include <string.h>
#include <lwiot.h>

#include <lwiot/log.h>

#include <lwiot/io/spibus.h>
#include <lwiot/io/spimessage.h>

#include <driver/spi_master.h>
#include <driver/spi_common.h>

#include <lwiot/stl/vector.h>

namespace lwiot
{
	namespace esp32
	{
		class SpiBus : public lwiot::SpiBus {
		public:
			explicit SpiBus(int mosi, int miso, int clk, uint32_t freq = 100000);
			~SpiBus() override = default;

			void setFrequency(uint32_t freq) override;
			bool transfer(SpiMessage& msg) override ;
			bool transfer(stl::Vector<SpiMessage>& msg) override ;

		private:
			spi_bus_config_t _config;

			static constexpr int SPI_DMA_CHANNEL = 1;
			static constexpr int SPI_QUEUE_SIZE = 7;
			static constexpr int SPI_MODE = 0;

			/* Methods */
			spi_device_handle_t setup(const SpiMessage& msg);
			void stop(spi_device_handle_t spi);
		};
	}
}
