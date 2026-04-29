
#ifndef _CD4066B_H_
#define _CD4066B_H_

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CD4066B switch channel enumeration
 */
enum cd4066b_channel {
	CD4066B_CHANNEL_TX = 0,
	CD4066B_CHANNEL_RX,
	CD4066B_CHANNEL_COUNT
};

/**
 * @brief CD4066B switch state enumeration
 */
enum cd4066b_state {
	CD4066B_SWITCH_ON = 0,
	CD4066B_SWITCH_OFF  = 1
};

/**
 * @brief CD4066B Interconnect signal(event) state 
 */
typedef enum cd4066b_event {

	CD4066B_EVENT_DETECTED = 0,
	CD4066B_EVENT_RESET  = 1

}cd4066b_event_t;
/**
 * @brief Callback function type for signal input events
 *
 * @param dev Pointer to the CD4066B device
 * @param user_data User-provided data pointer
 */
typedef void (*cd4066b_sig_callback_t)(const struct device *dev, cd4066b_event_t event, void *user_data);

/**
 * @brief CD4066B driver API structure
 */
struct cd4066b_api {
	int (*set_switch)(const struct device *dev, enum cd4066b_channel channel,
			  enum cd4066b_state state);
	int (*get_switch)(const struct device *dev, enum cd4066b_channel channel,
			  enum cd4066b_state *state);
	int (*set_callback)(const struct device *dev, cd4066b_sig_callback_t callback,
			    void *user_data);
	int (*get_sig_in_state)(const struct device *dev, int *state);
};

/**
 * @brief Set the state of a switch channel
 *
 * @param dev Pointer to the CD4066B device
 * @param channel Switch channel (TX or RX)
 * @param state Switch state (ON or OFF)
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_set_switch(const struct device *dev,
				     enum cd4066b_channel channel,
				     enum cd4066b_state state)
{
	const struct cd4066b_api *api = (const struct cd4066b_api *)dev->api;

	if (api->set_switch == NULL) {
		return -ENOTSUP;
	}
	return api->set_switch(dev, channel, state);
}

/**
 * @brief Get the current state of a switch channel
 *
 * @param dev Pointer to the CD4066B device
 * @param channel Switch channel (TX or RX)
 * @param state Pointer to store the switch state
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_get_switch(const struct device *dev,
				     enum cd4066b_channel channel,
				     enum cd4066b_state *state)
{
	const struct cd4066b_api *api = (const struct cd4066b_api *)dev->api;

	if (api->get_switch == NULL) {
		return -ENOTSUP;
	}
	return api->get_switch(dev, channel, state);
}

/**
 * @brief Enable TX switch
 *
 * @param dev Pointer to the CD4066B device
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_tx_enable(const struct device *dev)
{
	return cd4066b_set_switch(dev, CD4066B_CHANNEL_TX, CD4066B_SWITCH_ON);
}

/**
 * @brief Disable TX switch
 *
 * @param dev Pointer to the CD4066B device
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_tx_disable(const struct device *dev)
{
	return cd4066b_set_switch(dev, CD4066B_CHANNEL_TX, CD4066B_SWITCH_OFF);
}

/**
 * @brief Enable RX switch
 *
 * @param dev Pointer to the CD4066B device
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_rx_enable(const struct device *dev)
{
	return cd4066b_set_switch(dev, CD4066B_CHANNEL_RX, CD4066B_SWITCH_ON);
}

/**
 * @brief Disable RX switch
 *
 * @param dev Pointer to the CD4066B device
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_rx_disable(const struct device *dev)
{
	return cd4066b_set_switch(dev, CD4066B_CHANNEL_RX, CD4066B_SWITCH_OFF);
}

/**
 * @brief Set callback for signal input events
 *
 * @param dev Pointer to the CD4066B device
 * @param callback Callback function (NULL to disable)
 * @param user_data User data passed to callback
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_set_callback(const struct device *dev,
				       cd4066b_sig_callback_t callback,
				       void *user_data)
{
	const struct cd4066b_api *api = (const struct cd4066b_api *)dev->api;

	if (api->set_callback == NULL) {
		return -ENOTSUP;
	}
	return api->set_callback(dev, callback, user_data);
}


/**
 * @brief Get current state of signal input pin
 *
 * @param dev Pointer to the CD4066B device
 * @param state Pointer to store the pin state (0 or 1)
 * @return 0 on success, negative errno on failure
 */
static inline int cd4066b_get_sig_in_state(const struct device *dev, int *state)
{
	const struct cd4066b_api *api = (const struct cd4066b_api *)dev->api;

	if (api->get_sig_in_state == NULL) {
		return -ENOTSUP;
	}
	return api->get_sig_in_state(dev, state);
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_DRIVERS_MISC_CD4066B_H_ */
