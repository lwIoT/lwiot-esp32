
#include <stdlib.h>
#include <stdio.h>
#include <esp_attr.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_heap_caps.h>
#include <lwiot.h>

#include <lwiot/kernel/thread.h>
#include <lwiot/kernel/functionalthread.h>

#include <lwiot/io/gpiochip.h>
#include <lwiot/io/gpiopin.h>
#include <lwiot/io/watchdog.h>
#include <lwiot/io/file.h>

#include <lwiot/util/datetime.h>
#include <lwiot/util/application.h>

#include <lwiot/stl/vector.h>
#include <lwiot/stl/string.h>

#include <lwiot/esp32/spiffs.h>

#include <lwiot/log.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include <mbedtls/md5.h>

static const char *TAG = "spiffs-app";

class SpiFFSApplication : public lwiot::Functor {
private:
	static void read_hello_txt()
	{
		lwiot::File file("/storage/hello.txt", lwiot::FileMode::Read);
		ESP_LOGI(TAG, "Reading hello.txt");

		auto str = lwiot::stl::move(file.readString());
		print_dbg("Read from hello.txt (%i): %s\n", str.length(), str.c_str());

	}

	static void compute_alice_txt_md5()
	{
#define MD5_MAX_LEN 16
		lwiot::File file("/storage/sub/alice.txt", lwiot::FileMode::Read);
		mbedtls_md5_context ctx;
		unsigned char digest[MD5_MAX_LEN];
		size_t read;

		mbedtls_md5_init(&ctx);
		mbedtls_md5_starts_ret(&ctx);

		auto data = file.readString();
		read = data.length();
		mbedtls_md5_update_ret(&ctx, (unsigned const char*) data.c_str(), read);

		mbedtls_md5_finish_ret(&ctx, digest);

		char digest_str[MD5_MAX_LEN * 2];

		for (int i = 0; i < MD5_MAX_LEN; i++) {
			sprintf(&digest_str[i * 2], "%02x", (unsigned int)digest[i]);
		}

		print_dbg("Computed MD5 hash of alice.txt: %s\n", digest_str);
	}

protected:
	void run() override
	{
		lwiot::esp32::SpiFFS fs;

		ESP_LOGI(TAG, "Initializing SPIFFS");
		fs.mount(false);

		size_t total = fs.size();
		size_t used = fs.used();

		ESP_LOGI(TAG, "Partition size: total: %dKiB, used: %dKiB", total / 1024, used / 1024);

		read_hello_txt();

		compute_alice_txt_md5();

		fs.unmount();
		ESP_LOGI(TAG, "SPIFFS unmounted");
	}
};

extern "C" void main_start(void)
{
	SpiFFSApplication runner;
	lwiot::Application app(runner);

	app.start();
}
