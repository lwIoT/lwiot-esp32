/*
 * ESP32 WiFi station mode implementation.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <lwiot.h>

#include <lwip/ip_addr.h>
#include <lwiot/log.h>
#include <lwiot/network/wifistation.h>
#include <lwiot/types.h>

#include <lwip/err.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <lwip/dns.h>

#include <lwiot/esp32/esp32sta.h>

extern "C" {
extern void esp32_wifi_subsys_init(void);
extern void esp32_wifi_init_softap(const char *ssid, const char *pass, int max, uint8_t hidden, int channel);
extern void esp32_wifi_init_station(const char *ssid, const char *pass);
}

extern "C" void esp32_wifi_station_event(system_event_t *event)
{
	auto& station = lwiot::esp32::WifiStation::instance();
	auto addr = lwiot::IPAddress((uint32_t)0);

	switch((int)event->event_id) {
	case SYSTEM_EVENT_STA_GOT_IP:
		addr = lwiot::IPAddress((uint32_t)event->event_info.got_ip.ip_info.ip.addr);
		station.setStatus(lwiot::WL_CONNECTED);
		station.setAddress(addr);
		break;

	case SYSTEM_EVENT_STA_DISCONNECTED:
		station.setStatus(lwiot::WL_DISCONNECTED);
		station.setAddress(addr);
		break;

	default:
		break;
	}
}

namespace lwiot
{
	namespace esp32
	{
		WifiStation::WifiStation() : lwiot::WifiStation()
		{
			esp32_wifi_subsys_init();
		}

		void WifiStation::connectTo(const String &ssid)
		{
			esp32_wifi_init_station(ssid.c_str(), "");
		}

		void WifiStation::connectTo(const String &ssid, const String &pass)
		{
			esp32_wifi_init_station(ssid.c_str(), pass.c_str());
		}

		void WifiStation::setAddress(const IPAddress &addr)
		{
			lwiot::WifiStation::setAddress(addr);
		}

		void WifiStation::setStatus(lwiot::wireless_status_t status)
		{
			lwiot::WifiStation::setStatus(status);
		}

		void WifiStation::disconnect()
		{
			wifi_mode_t mode;

			esp_wifi_get_mode(&mode);
			if(mode == WIFI_MODE_APSTA)
				esp_wifi_set_mode(WIFI_MODE_AP);
			else
				esp_wifi_set_mode(WIFI_MODE_NULL);
		}

		WifiStation::operator bool() const
		{
			return this->_status == WL_CONNECTED;
		}
	}
}
