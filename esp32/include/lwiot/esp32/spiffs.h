/*
 * ESP32 SPIFFS wrapper.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#pragma once

#include <stdlib.h>
#include <lwiot.h>
#include <esp_spiffs.h>

#include <lwiot/stl/string.h>

namespace lwiot
{
	namespace esp32
	{
		class SpiFFS {
		public:
			explicit SpiFFS(stl::String mount = "/storage", int max = 4);
			virtual ~SpiFFS();

			bool mount(bool format_failure);
			void unmount();
			bool mounted();

			size_t size() const;
			size_t used() const;

		private:
			esp_vfs_spiffs_conf_t _config;

			stl::String _mount;
			int _max;
			bool _mounted;
		};
	}
}
