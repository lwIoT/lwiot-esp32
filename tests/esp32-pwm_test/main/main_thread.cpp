/*
 * ESP32 device test.
 * 
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <lwiot.h>

#include <lwiot/kernel/thread.h>
#include <lwiot/util/application.h>

#include <lwiot/io/pwm.h>
#include <lwiot/io/leddriver.h>

#include <lwiot/esp32/esp32pwm.h>

class EspApplication : public lwiot::Functor {
public:
	explicit EspApplication()
	{
	}

	virtual ~EspApplication()
	{
	}

protected:
	void run() override
	{
		lwiot::esp32::PwmTimer timer(0, MCPWM_UNIT_0, 100);
		auto& channel = timer[0];
		auto pin = lwiot::GpioPin(27);
		bool up = true;

		channel.setGpioPin(pin);
		channel.setDutyCycle(0);
		channel.enable();

		lwiot::LedDriver led(channel);

		print_dbg("Main thread started!\n");

		while(true) {
			if(up) {
				led.fade(100.0, 10);
			} else {
				led.fade(0.0, 10);
			}

			up = !up;
			print_dbg("PING!\n");
			//lwiot_sleep(1000);
		}
	}
};

extern "C" void main_start(void)
{
	EspApplication runner;
	lwiot::Application app(runner);

	app.start();
}

