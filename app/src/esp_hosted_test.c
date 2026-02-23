
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/sys/util_macro.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(wifi_test, LOG_LEVEL_INF);

/* Change these to your local network credentials */
#define SSID "get_hotspot"
#define PSK  "Your_Password"

static struct net_mgmt_event_callback wifi_cb;

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint64_t mgmt_event,
                               struct net_if *iface){

    switch (mgmt_event) {

    case NET_EVENT_WIFI_CONNECT_RESULT: {

	const struct wifi_status *status = (const struct wifi_status *)cb->info;
	if (status->status) {
		LOG_ERR("Connection failed (%d)", status->status);
	} else {
		LOG_INF("Connected successfully!");
	}
        break;
    }
    case NET_EVENT_IPV4_ADDR_ADD:
	break;

    case NET_EVENT_WIFI_DISCONNECT_RESULT:
	LOG_INF("WIFI disconnected!");
        break;
		//   case NET_EVENT_WIFI_SCAN_RESULT:
		// break;
		//   case NET_EVENT_WIFI_SCAN_DONE:
		// break;
    }
}

int main(void) {

	LOG_INF("Starting ESP-Hosted SDIO Wi-Fi Test...");

	/*Setup event callbacks so we know when we are connected */
	net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler,NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_IPV4_ADDR_ADD | NET_EVENT_WIFI_DISCONNECT_RESULT);
	net_mgmt_add_event_callback(&wifi_cb);

	struct net_if *iface = net_if_get_default();
	if(!iface) {

	    LOG_ERR("No network interface found!");
	    return -1;
	}

	struct wifi_connect_req_params params = {
		.ssid = SSID,
		.ssid_length = strlen(SSID),
		.psk = PSK,
		.psk_length = strlen(PSK),
		.channel = WIFI_CHANNEL_ANY,
		.security = WIFI_SECURITY_TYPE_WPA_PSK,
	};

	LOG_INF("Connecting to %s...", SSID);

	/* Trigger the connection */
	/* This calls your driver's .connect function via the Zephyr API */
	if(net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params))) {
	    LOG_ERR("Connection request failed");
	    return -1;
	}

    k_sleep(K_FOREVER);
    return 0;
}
