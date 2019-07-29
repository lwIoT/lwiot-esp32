/*
 * ESP32 device test.
 * 
 * @author Michel Megens
 * @email  dev@bietje.net
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <esp_heap_caps.h>
#include <lwiot.h>

#include <lwiot/kernel/thread.h>
#include <lwiot/kernel/dispatchqueue.h>
#include <lwiot/kernel/eventqueue.h>
#include <lwiot/util/application.h>

#include <lwiot/io/watchdog.h>

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
		lwiot::DispatchQueue<bool(int), lwiot::MultiThreadingPolicy> dpq;
		lwiot::EventQueue<bool(time_t)> evq;

		dpq.enable();

		evq.on("free", [](time_t time) {
			auto freesize = heap_caps_get_free_size(MALLOC_CAP_8BIT) / 1024;
			print_dbg("Free memory: %uKiB\n", freesize);
			return true;
		});

		evq.on("event", [](time_t time) {
			print_dbg("Event queue \"event\" triggered! \n");
			return true;
		});

		evq.enable();

		while(true) {
			dpq.enqueue([](int x) -> bool {
				print_dbg("Work item %i done!\n", x);
				return true;
			}, 4);

			dpq.enqueue([](int x) -> bool {
				print_dbg("Work item %i done!\n", x);
				return true;
			}, 5);

			evq.signal("free");
			evq.signal("event");

			lwiot::Thread::sleep(1000);
		}
	}
};

extern "C" void main_start(void)
{
	EspApplication runner;
	lwiot::Application app(runner);

	app.start();
}

