/*
 * XBee network test.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <esp_attr.h>
#include <lwiot.h>

#include <lwiot/kernel/thread.h>
#include <lwiot/stl/string.h>
#include <lwiot/io/uart.h>
#include <lwiot/stl/string.h>
#include <lwiot/io/watchdog.h>

#include <lwiot/network/stdnet.h>

#include <lwiot/network/xbee/xbeeaddress.h>
#include <lwiot/network/xbee/xbeeresponse.h>
#include <lwiot/network/xbee/xbeerequest.h>
#include <lwiot/network/xbee/asyncxbee.h>

#include <lwiot/esp32/esp32uart.h>

using namespace lwiot;

class MainThread : public lwiot::Thread {
public:
	explicit MainThread(const char *arg) : Thread("main-thread", (void *) arg)
	{
	}

protected:
	lwiot::AsyncXbee _xb;

	void handler(XBeeResponse& response)
	{
		lwiot::Rx64Response rx64;
		lwiot::Rx16Response rx16;
		lwiot::ZBRxResponse rxZB;

		print_dbg("Packet received!\n");
		if (response.isAvailable()) {
			if(response.getApiId() == RX_16_RESPONSE || response.getApiId() == RX_64_RESPONSE ||
					response.getApiId() == ZB_RX_RESPONSE) {
				uint8_t *data;

				if (response.getApiId() == RX_16_RESPONSE) {
					response.getRx16Response(rx16);
					data = rx16.getData();
				} else if(response.getApiId() == ZB_RX_RESPONSE) {
					response.getZBRxResponse(rxZB);
					data = rxZB.getData();
				} else {
					response.getRx64Response(rx64);
					data = rx64.getData();
				}

				print_dbg("Received: %c%c%c\n", data[0], data[1], data[2]);
			} else {
				print_dbg("Unexpected response..: %X\n", response.getApiId());
			}
		} else if (response.isError()) {
			print_dbg("Xbee error: %u\n", response.getErrorCode());
		}
	}

	void run()
	{
		uint8_t link_key[] = {0xAA, 0xBB, 0xCC};
		lwiot::esp32::Uart uart(2, 9600);
		lwiot::XBee xbee;
		lwiot::AsyncXbee::ResponseHandler func([&](XBeeResponse& resp) { this->handler(resp); });
		lwiot::ByteBuffer link(sizeof(link_key)), network;
		lwiot::ByteBuffer txdata;

		printf("Main thread started!\n");

		xbee.begin(uart);
		this->_xb.setDevice(xbee);
		this->_xb.begin(stl::move(func));

		Thread::sleep(1000);

		this->_xb.setNetworkID(0xBBAA);
		Thread::sleep(100);
		this->_xb.setChannel(0x200);

		link.write(link_key, sizeof(link_key));
		this->_xb.setNetworkKey(network);
		this->_xb.setLinkKey(link);
		this->_xb.setEncryptionOptions(lwiot::XBee::EncryptionOptions::TrustCenter);
		this->_xb.enableEncryption(true);
		this->_xb.setMaxHops(0x2E);
		this->_xb.setNodeIdentifier("COORD");
		this->_xb.enableCoordinator(true);

		this->_xb.writeToFlash();

		wdt.disable();

		txdata.write('B');
		txdata.write('C');
		txdata.write('F');

		while(true) {
			print_dbg("Network address: 0x%X\n", this->_xb.getNetworkAddress());
			print_dbg("Parent address: 0x%X\n", this->_xb.getParentAddress());
			this->_xb.transmit(ZigbeeAddress(0x602F), txdata);
			print_dbg("PING!\n");
			Thread::sleep(200);
		}
	}
};

static MainThread *mt;
const char *arg = "Hello, World! [FROM main-thread]";

extern "C" void main_start(void)
{
	printf("Creating main thread..");
	mt = new MainThread(arg);
	printf("[DONE]\n");
	mt->start();
}

