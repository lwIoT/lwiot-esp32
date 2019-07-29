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
#include <esp_core_dump.h>

#include <lwiot/kernel/thread.h>
#include <lwiot/log.h>
#include <lwiot/stl/string.h>
#include <lwiot/io/gpiochip.h>
#include <lwiot/io/gpiopin.h>
#include <lwiot/io/watchdog.h>
#include <lwiot/util/datetime.h>
#include <lwiot/util/guid.h>
#include <lwiot/io/gpioi2calgorithm.h>
#include <lwiot/io/i2cbus.h>
#include <lwiot/io/i2cmessage.h>
#include <lwiot/kernel/atomic.h>
#include <lwiot/io/hardwarei2calgorithm.h>
#include <lwiot/device/apds9301sensor.h>
#include <lwiot/device/dsrealtimeclock.h>

#include <lwiot/network/httpserver.h>
#include <lwiot/network/socketudpserver.h>
#include <lwiot/network/udpserver.h>
#include <lwiot/network/udpclient.h>
#include <lwiot/network/dnsclient.h>
#include <lwiot/network/sockettcpserver.h>
#include <lwiot/network/sockettcpclient.h>
#include <lwiot/network/wifiaccesspoint.h>
#include <lwiot/network/wifistation.h>
#include <lwiot/network/dnsserver.h>
#include <lwiot/network/ntpclient.h>
#include <lwiot/network/socketudpclient.h>

#include <lwiot/network/asyncmqttclient.h>

#include <lwiot/stl/move.h>

#include <lwiot/esp32/esp32pwm.h>
#include <lwiot/esp32/hardwarei2calgorithm.h>
#include <lwiot/esp32/esp32ap.h>
#include <lwiot/esp32/esp32watchdog.h>
#include <lwiot/esp32/esp32sta.h>

static double luxdata = 0x0;
static int a = 0;
static int b = 0;

class HttpServerThread : public lwiot::Thread {
public:
	explicit HttpServerThread(const char *arg) : Thread("http-thread", (void *) arg)
	{
	}

protected:
	void run() override
	{
		auto srv = new lwiot::SocketTcpServer(lwiot::IPAddress(192, 168, 1, 1), 80);
		lwiot::HttpServer server(srv);

		this->setup_server(server);
		server.begin();

		while(true) {
			server.handleClient();
			wdt.reset();
		}
	}

	void setup_server(lwiot::HttpServer &server)
	{
		server.on("/", [](lwiot::HttpServer &_server) -> void {
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
	explicit MainThread(const char *arg) : Thread("main-thread", (void *) arg)
	{
	}

protected:
	void startPwm(lwiot::esp32::PwmTimer &timer)
	{
		auto &channel = timer[0];
		auto pin = lwiot::GpioPin(27);

		channel.setGpioPin(pin);
		channel.setDutyCycle(75.0f);
		channel.enable();
		lwiot_sleep(2000);

		channel.setDutyCycle(15.0f);
		channel.reload();
	}

	void startAP(const lwiot::String &ssid, const lwiot::String &passw)
	{
		auto &ap = lwiot::esp32::WifiAccessPoint::instance();
		lwiot::IPAddress local(192, 168, 1, 1);
		lwiot::IPAddress subnet(255, 255, 255, 0);
		lwiot::IPAddress gw(192, 168, 1, 1);

		ap.start();
		ap.config(local, gw, subnet);
		ap.begin(ssid, passw, 8, false, 4);
	}

	void startStation()
	{
		auto &sta = lwiot::esp32::WifiStation::instance();
		sta.connectTo("Intranet", "Plofkipbanaan01");
		//sta.connectTo("bietje", "banaan01");
		while(sta.status() != lwiot::WL_CONNECTED) {
			wdt.reset();
			lwiot_sleep(100);
		}
	}

	void subscribe(lwiot::AsyncMqttClient &mqtt)
	{
		print_dbg("Subscribing...\n");
		auto result = mqtt.subscribe("test/subscribe/1", [&](const lwiot::ByteBuffer &payload) {
			lwiot::stl::String str(payload);

			a++;
			print_dbg("Data (subscribe/1): %s\n", str.c_str());
		}, lwiot::MqttClient::QOS0);

		result = result && mqtt.subscribe("test/subscribe/2", [&](const lwiot::ByteBuffer &payload) {
			lwiot::stl::String str(payload);

			b++;
			print_dbg("Data (subscribe/2): %s\n", str.c_str());
		}, lwiot::MqttClient::QOS0);

		if(!result) {
			print_dbg("Unable to subscribe!\n");
		}
	}

	virtual void run() override
	{
		lwiot::esp32::PwmTimer timer(0, MCPWM_UNIT_0, 100);
		lwiot::DateTime dt(1539189832);
		lwiot::I2CBus bus(new lwiot::esp32::HardwareI2CAlgorithm(22, 23, 400000));
		lwiot::IPAddress local(192, 168, 1, 1);
		lwiot::AsyncMqttClient mqtt(3000);
		lwiot::NtpClient ntp;
		lwiot::atomic_uint32_t count(0);
		lwiot::Guid guid;
		auto& WDT = lwiot::esp32::Watchdog::instance();
		lwiot::DnsClient dnsclient;

		lwiot_sleep(1000);
		wdt.disable();

		this->startPwm(timer);
		printf("Main thread started!\n");

		print_dbg("Time: %s\n", dt.toString().c_str());
		lwiot::Function<void()> recon_handler = [&]() {
			this->subscribe(mqtt);
			print_dbg("Reconnect handler!\n");
			count++;
		};

		mqtt.setReconnectHandler(recon_handler);

		this->startStation();
		this->startAP("lwIoT test", "testap1234");

		a = b = 0;

		lwiot::SocketTcpClient client(dnsclient.lookup("mail.sonatolabs.com"), 1883);
		client.setTimeout(2000);
		mqtt.start(client);

		if(!mqtt.connect(guid.toString(), "test", "test")) {
			print_dbg("Failed to connect to MQTT\n");
		}

		while(!mqtt.connected()) {
			print_dbg("Waiting for MQTT to connect!\n");
			Thread::sleep(1000);
		}

		lwiot_sleep(100);

		this->subscribe(mqtt);

		lwiot::SocketUdpServer *udp = new lwiot::SocketUdpServer(BIND_ADDR_ANY, 53);
		lwiot::DnsServer dns;

		dns.map("smartsensor.local", local);
		dns.map("www.smartsensor.local", local);

		udp->bind();
		dns.begin(udp);

		lwiot::Apds9301Sensor sensor(bus);
		sensor.begin();
		auto http = new HttpServerThread(nullptr);
		http->start();

		wdt.enable(7000);

		while(true) {
			WDT.reset();

			if(lwiot::esp32::WifiStation::instance().status() != lwiot::WL_CONNECTED) {
				this->startStation();
				WDT.reset();
				continue;
			}

			print_dbg("Fetching time..\n");
			WDT.disableLocal();
			auto addr = lwiot::stl::move(dnsclient.lookup("nl.pool.ntp.org"));
			WDT.reset();

			auto update_ok = false;
			lwiot::SocketUdpClient udp_client;

			if(addr != lwiot::IPAddress(0,0,0,0)) {
				print_dbg("NTP started..\n");
				udp_client.begin(addr, 123);

				WDT.reset();
				udp_client.setTimeout(700);
				ntp.begin(udp_client);
				update_ok = ntp.update();
			} else {
				print_dbg("NTP DNS lookup failed!\n");
			}

			WDT.reset();

			lwiot::DateTime now(ntp.time());

			if(!update_ok) {
				print_dbg("Unable to update time!\n");
			} else {
				print_dbg("TIme fetched!\n");
			}

			wdt.reset();

			sensor.getLux(luxdata);
			print_dbg("[%s]: Lux value: %f\n", now.toString().c_str(), luxdata);

			auto freesize = heap_caps_get_free_size(MALLOC_CAP_8BIT) / 1024;
			print_dbg("[%lu] PING: free memory: %uKiB\n", lwiot_tick_ms(), freesize);
			print_dbg("Reconnects: %u\n", count.load());
			wdt.reset();

			lwiot::stl::String msg("Free memory: ");
			msg += freesize;
			msg += "KiB - Reconnects: ";
			msg += count.load();

			mqtt.publish("test/subscribe/1", msg, false);
			wdt.reset();
			mqtt.publish("test/subscribe/2", "Test message for s2", false);
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


