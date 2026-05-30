
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/cd4066b.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Get device reference from device tree */
#define CD4066B_NODE DT_NODELABEL(cd4066b)


static const struct device *cd4066b_dev = DEVICE_DT_GET(CD4066B_NODE);

/* Semaphore to signal main thread from interrupt callback */
static K_SEM_DEFINE(sig_in_sem, 0, 1);

/**
 * @brief Signal input falling edge callback function
 *
 * Called from system workqueue when falling edge is detected
 */
static void event_callback(const struct device *dev, cd4066b_event_t event, void *user_data)
{
	ARG_UNUSED(user_data);

	if(event == CD4066B_EVENT_DETECTED) {

		LOG_INF("Smoke event detected");
	} else {

		LOG_INF("Smoke event Reset");
	}
	/* Signal main thread */
	k_sem_give(&sig_in_sem);
}



/**
 * @brief Main application entry point
 */
int main(void)
{
	int ret;

	enum cd4066b_state state;
	/* Check device is ready */
	if(!device_is_ready(cd4066b_dev)) {
		LOG_ERR("CD4066B device not ready");
		return -ENODEV;
	}

	cd4066b_rx_enable(cd4066b_dev);
	LOG_INF("Enabling TX switch After 10 seconds");
	k_msleep(10000);
	cd4066b_rx_disable(cd4066b_dev);
	cd4066b_tx_enable(cd4066b_dev);
	k_msleep(100);
	cd4066b_get_switch(cd4066b_dev, CD4066B_CHANNEL_TX, &state);
	LOG_INF("TX switch state: %s", state == CD4066B_SWITCH_ON ? "ON" : "OFF");
	LOG_INF("Disabling TX switch After 30 seconds");
	k_msleep(30000);
	cd4066b_tx_disable(cd4066b_dev);
	LOG_INF("TX switch state: %s", state == CD4066B_SWITCH_ON ? "ON" : "OFF");
	// /* Demonstrate switch control */
	// demonstrate_switch_control();
	//
	// ret = cd4066b_set_callback(cd4066b_dev, signal_falling_edge_callback, NULL);
	// if(ret < 0) {
	// 	LOG_ERR("Failed to set callback: %d", ret);
	// 	return ret;
	// }
	/* Main loop - wait for signal events */
	while (1) {
		k_msleep(60000);

		/* Wait for falling edge event (with timeout) */
		// ret = k_sem_take(&sig_in_sem, K_SECONDS(5));
		//
		// if(ret == 0) {
		// 	int sig_state;
		//
		// 	/* Event occurred - read current pin state */
		// 	cd4066b_get_sig_in_state(cd4066b_dev, &sig_state);
		// 	LOG_INF("Current signal input state: %d", sig_state);
		//
		// 	/* Example: Enable RX when falling edge detected */
		// 	cd4066b_rx_enable(cd4066b_dev);
		//
		// 	/* Do some processing... */
		// 	k_msleep(100);
		//
		// 	/* Disable RX after processing */
		// 	cd4066b_rx_disable(cd4066b_dev);
		// } else {
		// 	/* Timeout - print status */
		// 	LOG_DBG("No falling edge in last 5 seconds (total events: %u)", 
		// 		event_count);
		// }
	}
	return 0;
}
