/*
 * ESP32 WiFi station mode definition.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <lwiot.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <tcpip_adapter.h>

#include <lwiot/log.h>
#include <lwiot/types.h>

#include <lwiot/network/ipaddress.h>
#include <lwiot/network/wifistation.h>

namespace lwiot
{
	namespace esp32
	{
		class WifiStation : public lwiot::WifiStation {
		public:
			static WifiStation& instance()
			{
				static WifiStation sta;
				return sta;
			}

			WifiStation(const WifiStation&) = delete;
			void operator =(const WifiStation&) = delete;

			void connectTo(const String& ssid) override;
			void connectTo(const String& ssid, const String& password) override;
			void disconnect() override;

			void setStatus(wireless_status_t status) override;
			void setAddress(const IPAddress& addr) override;

			explicit operator bool () const override;

		private:
			explicit WifiStation();
			virtual ~WifiStation() = default;
		};
	}
}
