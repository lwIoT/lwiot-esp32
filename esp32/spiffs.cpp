/*
 * SPIFFS wrapper implementation.
 *
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <string.h>
#include <lwiot.h>
#include <esp_spiffs.h>

#include <lwiot/log.h>
#include <lwiot/stl/string.h>
#include <lwiot/stl/move.h>

#include <lwiot/esp32/spiffs.h>

namespace lwiot
{
	namespace esp32
	{
		SpiFFS::SpiFFS(lwiot::String mount, int max) : _mount(stl::move(mount)), _max(max), _mounted(false)
		{
			this->_config.base_path = this->_mount.c_str();
			this->_config.max_files = max;
			this->_config.format_if_mount_failed = false;
			this->_config.partition_label = nullptr;
		}

		SpiFFS::~SpiFFS()
		{
			if(!this->_mounted)
				return;

			this->unmount();
		}

		bool SpiFFS::mount(bool format_failure)
		{
			esp_err_t ret;

			this->_config.format_if_mount_failed = format_failure;
			ret = esp_vfs_spiffs_register(&this->_config);

			if (ret != ESP_OK) {
				if (ret == ESP_FAIL) {
					print_dbg("Failed to mount or format filesystem");
				} else if (ret == ESP_ERR_NOT_FOUND) {
					print_dbg("Failed to find SPIFFS partition");
				} else {
					print_dbg("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
				}
			}

			this->_mounted = ret == ESP_OK;
			return this->_mounted;
		}

		size_t SpiFFS::size() const
		{
			esp_err_t ret;
			size_t total = 0, used = 0;

			ret = esp_spiffs_info(nullptr, &total, &used);
			if(ret != ESP_OK) {
				print_dbg("Unable to get SPIFFS stats!\n");
				return 0UL;
			}

			return total;
		}

		size_t SpiFFS::used() const
		{
			esp_err_t ret;
			size_t total = 0, used = 0;

			ret = esp_spiffs_info(nullptr, &total, &used);
			if(ret != ESP_OK) {
				print_dbg("Unable to get SPIFFS stats!\n");
				return 0UL;
			}

			return used;
		}

		void SpiFFS::unmount()
		{
			this->_mounted = false;
			esp_vfs_spiffs_unregister(NULL);
		}

		bool SpiFFS::mounted()
		{
			return this->_mounted;
		}
	}
}
