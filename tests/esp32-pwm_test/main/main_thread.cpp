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
#include <lwiot/io/rgbleddriver.h>
#include <lwiot/io/watchdog.h>

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
		lwiot::esp32::PwmTimer timer1(0, MCPWM_UNIT_0, 10000);
		lwiot::esp32::PwmTimer timer2(1, MCPWM_UNIT_0, 10000);

		auto& g_channel = timer1[0];
		auto& r_channel = timer1[1];
		auto& b_channel = timer2[0];

		wdt.enable(2000);

		auto gpin = lwiot::GpioPin(27);
		auto bpin = lwiot::GpioPin(12);
		auto rpin = lwiot::GpioPin(33);

		g_channel.setGpioPin(gpin);
		g_channel.setDutyCycle(0);
		g_channel.enable();

		b_channel.setGpioPin(bpin);
		b_channel.setDutyCycle(0);
		b_channel.enable();

		r_channel.setGpioPin(rpin);
		r_channel.setDutyCycle(0);
		r_channel.enable();

		lwiot::RgbLedDriver rgb(r_channel, g_channel, b_channel);
		/*lwiot::LedDriver g(g_channel);
		lwiot::LedDriver r(r_channel);
		lwiot::LedDriver b(b_channel);

		print_dbg("Main thread started!\n");

		r.setBrightness(85);
		g.setBrightness(96);
		b.setBrightness(25);

		lwiot_sleep(3000);

		r.setBrightness(0);
		g.setBrightness(100);*/

		rgb.set(200, 30, 210);
		lwiot_sleep(1000);
		rgb.fadeBrightness(true);
		rgb.fadeBrightness(false);

		lwiot_sleep(3000);

		while(true) {
			/*if(up) {
				g.fade(100.0, 30);
				r.fade(0.0, 30);
			} else {
				g.fade(0.0, 30);
				r.fade(100.0, 30);
			}

			up = !up;*/

			for(int idx = 0; idx < 3; idx++) {
				rgb.fade(50, 250, 20);
				wdt.reset();
				rgb.fade(200, 30, 210);
				wdt.reset();
				print_dbg("PING!\n");
			}

			print_dbg("FADE done!\n");

			rgb.setHSV(40, 1, 1);
			lwiot_sleep(1000);
			rgb.setHSV(40, 100, 100);
			wdt.reset();
			lwiot_sleep(1000);

			for(int idx = 0; idx < 2; idx++) {
				print_dbg("PONG!\n");
				rgb.rotate();
				wdt.reset();
			}

			print_dbg("Cycle complete\n");
			rgb.setHSV(219, 79, 70);

			wdt.reset();
			lwiot_sleep(1000);
			wdt.reset();
		}
	}
};

extern "C" void main_start(void)
{
	EspApplication runner;
	lwiot::Application app(runner);

	app.start();
}

