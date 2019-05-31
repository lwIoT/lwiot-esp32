/*
 * GPIO chip for the ESP32 chip.
 * 
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#pragma once

#include <lwiot/lwiot.h>
#include <lwiot/io/gpiochip.h>

#ifdef __cplusplus
namespace lwiot { namespace esp32
{
	class GpioChip : public lwiot::GpioChip {
	public:
		explicit GpioChip();
		virtual ~GpioChip() = default;

		void mode(int pin, const PinMode& mode) override;
		void write(int pin, bool value) override;
		bool read(int pin) const override;

		void setOpenDrain(int pin) override;
		void odWrite(int pin, bool value) override;
		void attachIrqHandler(int pin, irq_handler_t handler, IrqEdge edge) override;
		void detachIrqHandler(int pin) override;

	private:
		int mapIrqType(const IrqEdge& edge) const;

		bool _initializedIrqs;
	};
}
}
#endif
