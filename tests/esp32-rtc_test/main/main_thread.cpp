/*
 * ESP32 device test.
 * 
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <stdio.h>
#include <esp_attr.h>
#include <esp_heap_caps.h>
#include <lwiot.h>

#include <lwiot/kernel/thread.h>
#include <lwiot/stl/string.h>

#include <lwiot/log.h>
#include <lwiot/io/gpiochip.h>
#include <lwiot/io/gpiopin.h>
#include <lwiot/io/watchdog.h>
#include <lwiot/io/i2cbus.h>
#include <lwiot/io/hardwarei2calgorithm.h>
#include <lwiot/io/gpioi2calgorithm.h>

#include <lwiot/esp32/hardwarei2calgorithm.h>

#include <lwiot/device/dsrealtimeclock.h>
#include <lwiot/util/datetime.h>

static lwiot::DateTime now;
static volatile bool triggered = false;

static void timer_handler()
{
	triggered = true;
}

class MainThread : public lwiot::Thread {
public:
	explicit MainThread(const char *arg) : Thread("main-thread", (void *) arg)
	{
	}

protected:
	void run() override
	{
		lwiot::GpioPin scl(22), sda(21);
		auto algo = new lwiot::esp32::HardwareI2CAlgorithm(scl, sda, 400000U);
		lwiot::I2CBus bus(algo);
		lwiot::DsRealTimeClock rtc(bus);
		lwiot::DateTime dt(1500000000);

		lwiot_sleep(100); // Stabilize application

		//gpio.attachIrqHandler(23, timer_handler, lwiot::IrqEdge::IrqRising);
		gpio.attachIrqHandler(14, timer_handler, lwiot::IrqEdge::IrqFalling);

		rtc.set(dt);
		lwiot_sleep(10);
		rtc.setAlarm(lwiot::DsRealTimeClock::ALARM1_MATCH_SECONDS, 10, 0,0,2);
		rtc.alarm(lwiot::DsRealTimeClock::Alarm::ALARM_ONE);
		rtc.alarm(lwiot::DsRealTimeClock::Alarm::ALARM_TWO);
		rtc.enableAlarmInterrupt(lwiot::DsRealTimeClock::ALARM_ONE, true);

		wdt.enable(2000);

		while(true) {
			now = rtc.now();
			print_dbg("PING: %s\n", now.toString().c_str());

			if(triggered) {
				triggered = false;
				print_dbg("Timer triggered at: %s\n", now.toString().c_str());
			}

			if(rtc.alarm(lwiot::DsRealTimeClock::Alarm::ALARM_ONE)) {
				print_dbg("Alarm triggered!\n");
				rtc.setAlarm(lwiot::DsRealTimeClock::ALARM1_MATCH_SECONDS, 15);
			}

			wdt.reset();
			lwiot_sleep(1000);
		}
	}
};

static MainThread *mt;
static const char *arg = "Hello, World! [FROM main-thread]";

extern "C" void main_start(void)
{
	printf("Creating main thread..");
	mt = new MainThread(arg);
	printf(" [DONE]\n");
	mt->start();
}

