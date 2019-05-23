/*
 * ESP32 SPI bus implementation.
 * 
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <lwiot/io/spibus.h>
#include <lwiot/io/spimessage.h>
#include <lwiot/log.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>

struct spi_struct_t;
typedef struct spi_struct_t spi_t;

#define HSPI 2
#define VSPI 3

namespace lwiot { namespace esp32
{
	class SpiBus : public lwiot::SpiBus {
	public:
		explicit SpiBus(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t num = VSPI);
		virtual ~SpiBus();

		void setFrequency(uint32_t freq) override;
		bool transfer(SpiMessage &msg) override;

		using lwiot::SpiBus::transfer;

	protected:


	private:
		uint32_t _frequency;
		uint8_t _num;
		uint32_t _div;
		spi_t *_spi;
	};
}
}
