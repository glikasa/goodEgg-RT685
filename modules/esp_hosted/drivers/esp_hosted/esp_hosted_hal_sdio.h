/*
 * SPDX-License-Identifier: Apache-2.0
 * ESP Hosted SDIO HAL Interface
 */

#ifndef ESP_HOSTED_HAL_SDIO_H
#define ESP_HOSTED_HAL_SDIO_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <esp_hosted_wifi.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SDIO Configuration */
#define ESP_SDIO_BLOCK_SIZE         512
#define ESP_SDIO_FUNC_NUM           1

/* ESP SDIO Slave Register Map */
#define ESP_SLAVE_SCRATCH_REG_0     0x3FF5_3C00  /* Config/Status */
#define ESP_SLAVE_SCRATCH_REG_7     0x3FF5_3C1C  /* Capabilities */

/* SDIO Slave Register Offsets (Function 1) */
#define ESP_SDIO_INT_RAW_REG        0x04
#define ESP_SDIO_INT_ST_REG         0x08
#define ESP_SDIO_INT_CLR_REG        0x0C
#define ESP_SDIO_PKT_LEN_REG        0x10
#define ESP_SDIO_TOKEN_RDATA_REG    0x44
#define ESP_SDIO_CONF_REG           0x48

/* SDIO Data Port */
#define ESP_SDIO_SEND_OFFSET        0x10000
#define ESP_SDIO_RECV_OFFSET        0x10000

/* Interrupt Bits */
#define ESP_SDIO_INT_RX_NEW_PKT     BIT(0)
#define ESP_SDIO_INT_RX_UNDERFLOW   BIT(1)
#define ESP_SDIO_INT_RX_OVERFLOW    BIT(2)
#define ESP_SDIO_INT_TX_DONE        BIT(3)

/**
 * @brief Initialize SDIO HAL
 * @param dev Device instance
 * @return 0 on success, negative errno on failure
 */
int esp_hosted_hal_init(const struct device *dev);

/**
 * @brief Deinitialize SDIO HAL
 * @param dev Device instance
 * @return 0 on success, negative errno on failure
 */
int esp_hosted_hal_deinit(const struct device *dev);

/**
 * @brief Reset ESP device
 * @param dev Device instance
 * @return 0 on success, negative errno on failure
 */
int esp_hosted_hal_reset(const struct device *dev);

/**
 * @brief Transfer data over SDIO
 * @param dev Device instance
 * @param tx_frame Frame to send (NULL for RX only)
 * @param rx_frame Buffer for received frame (NULL for TX only)
 * @param len Maximum transfer length
 * @return 0 on success, negative errno on failure
 */
int esp_hosted_hal_sdio_transfer(const struct device *dev,
				 esp_frame_t *tx_frame,
				 esp_frame_t *rx_frame,
				 size_t len);

/**
 * @brief Check if RX data is available
 * @param dev Device instance
 * @param len Output: available data length
 * @return 0 if data available, -EAGAIN if not, other negative on error
 */
int esp_hosted_hal_data_ready(const struct device *dev, uint32_t *len);

/**
 * @brief Wait for data ready with timeout
 * @param dev Device instance
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on data ready, negative errno on timeout/error
 */
int esp_hosted_hal_wait_ready(const struct device *dev, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* ESP_HOSTED_HAL_SDIO_H */
