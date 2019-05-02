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
#include <lwiot/log.h>
#include <lwiot/stl/string.h>
#include <lwiot/io/gpiochip.h>
#include <lwiot/io/gpiopin.h>
#include <lwiot/io/watchdog.h>
#include <lwiot/util/datetime.h>
#include <lwiot/io/gpioi2calgorithm.h>
#include <lwiot/io/i2cbus.h>
#include <lwiot/io/i2cmessage.h>
#include <lwiot/io/hardwarei2calgorithm.h>
#include <lwiot/device/apds9301sensor.h>
#include <lwiot/device/dsrealtimeclock.h>

#include <lwiot/network/httpserver.h>
#include <lwiot/network/socketudpserver.h>
#include <lwiot/network/udpserver.h>
#include <lwiot/network/udpclient.h>
#include <lwiot/network/sockettcpserver.h>
#include <lwiot/network/wifiaccesspoint.h>
#include <lwiot/network/captiveportal.h>

#include <lwiot/stl/move.h>

#include <lwiot/esp32/esp32pwm.h>
#include <lwiot/esp32/esp32i2calgorithm.h>

static double luxdata = 0x0;

class HttpServerThread : public lwiot::Thread {
public:
	explicit HttpServerThread(const char *arg) : Thread("http-thread", (void*)arg)
	{
	}

protected:
	void run() override
	{
		auto srv = new lwiot::SocketTcpServer(lwiot::IPAddress(192,168,1,1), 8080);
		lwiot::HttpServer server(srv);

		this->setup_server(server);
		server.begin();

		while(true) {
			server.handleClient();
		}
	}

	void setup_server(lwiot::HttpServer& server)
	{
		server.on("/", [](lwiot::HttpServer& _server) -> void {
			char temp[500];
			snprintf(temp, 500,
			         "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 DEMO</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Lux value: %f</p>\
  </body>\
</html>", luxdata
			);

			_server.send(200, "text/html", temp);
		});
	}
};

#include <lwiot/kernel/eventqueue.h>

class MainThread : public lwiot::Thread {
public:
	explicit MainThread(const char *arg) : Thread("main-thread", (void*)arg)
	{
	}

protected:
	void startPwm(lwiot::esp32::PwmTimer& timer)
	{
		auto& channel = timer[0];
		auto pin = lwiot::GpioPin(5);

		channel.setGpioPin(pin);
		channel.setDutyCycle(75.0f);
		channel.enable();
		lwiot_sleep(2000);

		channel.setDutyCycle(50.0f);
		timer.setFrequency(100);
		channel.reload();
	}

	void startAP(const lwiot::String& ssid, const lwiot::String& passw)
	{
		auto& ap = lwiot::WifiAccessPoint::instance();
		lwiot::IPAddress local(192, 168, 1, 1);
		lwiot::IPAddress subnet(255, 255, 255, 0);
		lwiot::IPAddress gw(192, 168, 1, 1);

		ap.start();
		ap.config(local, gw, subnet);
		ap.begin(ssid, passw, 4);
	}

	virtual void run() override
	{
		lwiot::esp32::PwmTimer timer(0, MCPWM_UNIT_0, 100);
		lwiot::DateTime dt(1539189832);
		lwiot::I2CBus bus(new lwiot::HardwareI2CAlgorithm(22, 23, 400000));

		lwiot_sleep(1000);
		this->startPwm(timer);
		printf("Main thread started!\n");

		print_dbg("Time: %s\n", dt.toString().c_str());

		this->startAP("lwIoT test", "testap1234");

		lwiot::SocketUdpServer* udp = new lwiot::SocketUdpServer();
		lwiot::CaptivePortal portal(lwiot::IPAddress(192,168,1,1), lwiot::IPAddress(192,168,1,1));
		portal.begin(udp, 53);
		wdt.enable(2000);

		lwiot::Apds9301Sensor sensor(bus);
		sensor.begin();
		auto http = new HttpServerThread(nullptr);
		http->start();

		while(true) {
			wdt.reset();

			sensor.getLux(luxdata);

			auto freesize = heap_caps_get_free_size(MALLOC_CAP_DEFAULT) / 1024;
			printf("[%lu] PING: free memory: %uKiB\n", lwiot_tick_ms(), freesize);

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


