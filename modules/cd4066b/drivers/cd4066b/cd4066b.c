

#define DT_DRV_COMPAT ti_cd4066b

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/irq.h>
#include <zephyr/drivers/cd4066b.h>

LOG_MODULE_REGISTER(cd4066b, CONFIG_CD4066B_LOG_LEVEL);

/**
 * @brief Device configuration structure (ROM)
 */
struct cd4066b_config {
	struct gpio_dt_spec tx_gpio;
	struct gpio_dt_spec rx_gpio;
	struct gpio_dt_spec sig_in_gpio;
};

/**
 * @brief Device runtime data structure (RAM)
 */
struct cd4066b_data {
	const struct device *dev;
	struct gpio_callback sig_in_cb_data;
	cd4066b_sig_callback_t user_callback;
	void *user_data;
	cd4066b_event_t last_event;
	struct k_work callback_work;
	enum cd4066b_state tx_state;
	enum cd4066b_state rx_state;
	struct k_mutex lock;
};

/**
 * @brief Work handler to process signal input callbacks in system workqueue
 */
static void cd4066b_callback_work_handler(struct k_work *work){

	struct cd4066b_data *data = CONTAINER_OF(work, struct cd4066b_data, callback_work);

	if(data->user_callback != NULL) {
		data->user_callback(data->dev, data->last_event, data->user_data);
	}
}

/**
 * @brief GPIO interrupt callback for signal input (falling edge)
 */
static void cd4066b_sig_in_callback(const struct device *port,
				    struct gpio_callback *cb,
				    gpio_port_pins_t pins)
{
	struct cd4066b_data *data = CONTAINER_OF(cb, struct cd4066b_data, sig_in_cb_data);

	const struct cd4066b_config *config = data->dev->config;
	ARG_UNUSED(port);
	ARG_UNUSED(pins);

	LOG_DBG("Signal input event detected");
	int pin_state = gpio_pin_get_dt(&config->sig_in_gpio);

	data->last_event = pin_state ? CD4066B_EVENT_RESET : CD4066B_EVENT_DETECTED;
	/* Submit work to system workqueue for callback processing */
	k_work_submit(&data->callback_work);
}

/**
 * @brief Set the state of a switch channel
 */
static int cd4066b_set_switch_impl(const struct device *dev,
				   enum cd4066b_channel channel,
				   enum cd4066b_state state)
{
	const struct cd4066b_config *config = dev->config;
	struct cd4066b_data *data = dev->data;
	const struct gpio_dt_spec *gpio_spec;
	int ret;

	if (channel >= CD4066B_CHANNEL_COUNT) {
		LOG_ERR("Invalid channel: %d", channel);
		return -EINVAL;
	}

	k_mutex_lock(&data->lock, K_FOREVER);

	switch (channel) {
	case CD4066B_CHANNEL_TX:
		gpio_spec = &config->tx_gpio;
		break;
	case CD4066B_CHANNEL_RX:
		gpio_spec = &config->rx_gpio;
		break;
	default:
		k_mutex_unlock(&data->lock);
		return -EINVAL;
	}

	ret = gpio_pin_set_dt(gpio_spec, (int)state);
	if (ret < 0) {
		LOG_ERR("Failed to set GPIO pin: %d", ret);
		k_mutex_unlock(&data->lock);
		return ret;
	}

	/* Update cached state */
	switch (channel) {
	case CD4066B_CHANNEL_TX:
		data->tx_state = state;
		LOG_INF("TX switch %s", state == CD4066B_SWITCH_ON ? "ON" : "OFF");
		break;
	case CD4066B_CHANNEL_RX:
		data->rx_state = state;
		LOG_INF("RX switch %s", state == CD4066B_SWITCH_ON ? "ON" : "OFF");
		break;
	default:
		break;
	}

	k_mutex_unlock(&data->lock);
	return 0;
}

/**
 * @brief Get the current state of a switch channel
 */
static int cd4066b_get_switch_impl(const struct device *dev,
				   enum cd4066b_channel channel,
				   enum cd4066b_state *state)
{
	struct cd4066b_data *data = dev->data;

	if (channel >= CD4066B_CHANNEL_COUNT) {
		LOG_ERR("Invalid channel: %d", channel);
		return -EINVAL;
	}

	k_mutex_lock(&data->lock, K_FOREVER);

	switch (channel) {
	case CD4066B_CHANNEL_TX:
		*state = data->tx_state;
		break;
	case CD4066B_CHANNEL_RX:
		*state = data->rx_state;
		break;
	default:
		k_mutex_unlock(&data->lock);
		return -EINVAL;
	}

	k_mutex_unlock(&data->lock);
	return 0;
}

/**
 * @brief Set callback for signal input falling edge events
 */
static int cd4066b_set_callback_impl(const struct device *dev,
				     cd4066b_sig_callback_t callback,
				     void *user_data)
{
	struct cd4066b_data *data = dev->data;

	k_mutex_lock(&data->lock, K_FOREVER);

	data->user_callback = callback;
	data->user_data = user_data;

	LOG_DBG("Signal input callback %s", callback != NULL ? "set" : "cleared");

	k_mutex_unlock(&data->lock);
	return 0;
}

/**
 * @brief Get current state of signal input pin
 */
static int cd4066b_get_sig_in_state_impl(const struct device *dev, int *state)
{
	const struct cd4066b_config *config = dev->config;
	int ret;

	ret = gpio_pin_get_dt(&config->sig_in_gpio);
	if (ret < 0) {
		LOG_ERR("Failed to read signal input pin: %d", ret);
		return ret;
	}

	*state = ret;
	return 0;
}

/**
 * @brief Driver API structure
 */
static const struct cd4066b_api cd4066b_api_funcs = {
	.set_switch = cd4066b_set_switch_impl,
	.get_switch = cd4066b_get_switch_impl,
	.set_callback = cd4066b_set_callback_impl,
	.get_sig_in_state = cd4066b_get_sig_in_state_impl,
};

/**
 * @brief Device initialization function
 */
static int cd4066b_init(const struct device *dev)
{
	const struct cd4066b_config *config = dev->config;
	struct cd4066b_data *data = dev->data;
	int ret;

	LOG_INF("Initializing CD4066B driver");

	/* Store device reference */
	data->dev = dev;

	/* Initialize mutex */
	k_mutex_init(&data->lock);

	/* Initialize work item for callback processing */
	k_work_init(&data->callback_work, cd4066b_callback_work_handler);

	/* Initialize callback to NULL */
	data->user_callback = NULL;
	data->user_data = NULL;

	/* Initialize TX GPIO */
	if (!gpio_is_ready_dt(&config->tx_gpio)) {
		LOG_ERR("TX GPIO device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->tx_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure TX GPIO: %d", ret);
		return ret;
	}
	data->tx_state = CD4066B_SWITCH_OFF;
	LOG_DBG("TX GPIO configured on port %s pin %d", 
		config->tx_gpio.port->name, config->tx_gpio.pin);

	/* Initialize RX GPIO */
	if (!gpio_is_ready_dt(&config->rx_gpio)) {
		LOG_ERR("RX GPIO device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->rx_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure RX GPIO: %d", ret);
		return ret;
	}
	data->rx_state = CD4066B_SWITCH_OFF;
	LOG_DBG("RX GPIO configured on port %s pin %d",
		config->rx_gpio.port->name, config->rx_gpio.pin);

	/* Initialize Signal Input GPIO */
	if (!gpio_is_ready_dt(&config->sig_in_gpio)) {
		LOG_ERR("Signal input GPIO device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&config->sig_in_gpio, GPIO_INPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure signal input GPIO: %d", ret);
		return ret;
	}
	LOG_DBG("Signal input GPIO configured on port %s pin %d",
		config->sig_in_gpio.port->name, config->sig_in_gpio.pin);

	/* Setup GPIO callback for signal input */
	gpio_init_callback(&data->sig_in_cb_data,
			   cd4066b_sig_in_callback,
			   BIT(config->sig_in_gpio.pin));

	ret = gpio_add_callback(config->sig_in_gpio.port, &data->sig_in_cb_data);
	if (ret < 0) {
		LOG_ERR("Failed to add GPIO callback: %d", ret);
		return ret;
	}

	/* Enable falling edge interrupt - MANDATORY */
	// ret = gpio_pin_interrupt_configure_dt(&config->sig_in_gpio, GPIO_INT_EDGE_BOTH);
	// if (ret < 0) {
	// 	LOG_ERR("Failed to configure falling edge interrupt: %d", ret);
	// 	return ret;
	// }
	LOG_INF("CD4066B driver initialized");
	return 0;
}

/* Device instantiation macro */
#define CD4066B_DEVICE(inst)                                        \
	static struct cd4066b_data cd4066b_data_##inst;                 \
                                                                    \
	static const struct cd4066b_config cd4066b_config_##inst = {    \
		.tx_gpio = GPIO_DT_SPEC_INST_GET(inst, tx_gpios),           \
		.rx_gpio = GPIO_DT_SPEC_INST_GET(inst, rx_gpios),           \
		.sig_in_gpio = GPIO_DT_SPEC_INST_GET(inst, sig_in_gpios),   \
	};                                                              \
                                                                    \
	DEVICE_DT_INST_DEFINE(inst,                                     \
			      cd4066b_init,                                     \
			      NULL,                                             \
			      &cd4066b_data_##inst,                             \
			      &cd4066b_config_##inst,                           \
			      POST_KERNEL,                                      \
			      CONFIG_CD4066B_INIT_PRIORITY,                     \
			      &cd4066b_api_funcs);

DT_INST_FOREACH_STATUS_OKAY(CD4066B_DEVICE)
