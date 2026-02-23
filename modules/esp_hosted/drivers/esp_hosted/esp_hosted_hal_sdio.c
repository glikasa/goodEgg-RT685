/*
 * SPDX-License-Identifier: Apache-2.0
 * ESP Hosted SDIO HAL Implementation
 */

#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sd/sdio.h>
#include <zephyr/sd/sd_spec.h>
#include <esp_hosted_hal_sdio.h>
#include <esp_hosted_wifi.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(esp_hosted_hal, CONFIG_WIFI_LOG_LEVEL);

#define ESP_SDIO_REG_TOKEN_RDATA    0x044  /* TX credits available */
#define ESP_SDIO_REG_PKT_LEN        0x060  /* RX packet length available */
#define ESP_SDIO_REG_FIFO           0x1F800
#define ESP_ADDRESS_MASK            0x1FFFF

int esp_hosted_hal_init(const struct device *dev){

	const esp_hosted_config_t *config = dev->config;
	esp_hosted_data_t *data = dev->data;
	int ret;

	/* Reset the slave via GPIO (Standard for both SPI/SDIO) */
	gpio_pin_configure_dt(&config->reset_gpio, GPIO_OUTPUT_INACTIVE);
	gpio_pin_set_dt(&config->reset_gpio, 1); // Active Low Reset
	k_msleep(100);
	gpio_pin_set_dt(&config->reset_gpio, 0);
	k_msleep(500);

	if(!device_is_ready(config->sdhc_dev)) {
		
	    LOG_ERR("SDHC device is not ready");
	    return -ENODEV;
	}

	ret = sd_init(config->sdhc_dev, &data->card);
	if(ret) {

	    LOG_ERR("sd_init error: %x", ret);
	    return ret;
	}

	// ret = sdio_init_func(&data->card, &data->card.func0, SDIO_FUNC_NUM_0);
	// if(ret) {
	//
	//     LOG_ERR("sdio_enable_func BUS_FUNCTION, error: %x", ret);
	//     return ret;
	// }
	ret = sdio_init_func(&data->card, &data->sdio_func1, SDIO_FUNC_NUM_1);
	if(ret) {

	    LOG_ERR("sdio_enable_func FIFO, error: %x", ret);
	    return ret;
	}

	/*Initialize SDIO Function 1 */
	ret = sdio_enable_func(&data->sdio_func1);
	if(ret) {

	    LOG_ERR("Failed to enable SDIO Function 1 (%d)", ret);
	    return ret;
	}

	/*Set Block Size (ESP32 slave defaults to 512) */
	ret = sdio_set_block_size(&data->sdio_func1, 512);
	if(ret) {

	    LOG_ERR("Failed to set SDIO block size (%d)", ret);
	    return ret;
	}
	return 0;
}

/**
 * Replaces the GPIO-based data_ready.
 * In SDIO, we read the PKT_LEN register to see if a frame is waiting.
 */
// int esp_hosted_hal_data_ready(const struct device *dev){
//
// 	const esp_hosted_config_t *config = dev->config;
// 	struct sdio_func *func1 = config->sdhc_dev->data;
// 	uint8_t len_reg[4];
//
// 	/* Read Packet Len Register (0x060) */
// 	if (sdio_read_addr(func1, ESP_SDIO_REG_PKT_LEN, len_reg, 4) != 0) {
// 		return 0;
// 	}
// 	/* If any byte is non-zero, data is waiting */
// 	uint32_t len = (len_reg[0]) | (len_reg[1] << 8) | (len_reg[2] << 16);
// 	return (len > 0);
// }



int esp_hosted_hal_sdio_transfer(const struct device *dev,
				 esp_frame_t *tx_frame,
				 esp_frame_t *rx_frame,
				 size_t len) {


    int ret = 0;
    esp_hosted_data_t *data = dev->data;
    struct sdio_func *func1 = &data->sdio_func1;

    /* 1. Calculate Block count */
    /* ESP32 Slave requires 512-byte blocks for 'Extended Mode' */
    uint32_t block_size = 512;
    uint32_t blocks = len / block_size;
    uint32_t remainder = len % block_size;
    bool is_write = tx_frame != NULL;
    uint8_t *buf = is_write ? (uint8_t*)tx_frame : (uint8_t*)rx_frame;

    /*Wait for Flow Control*/
    if (k_sem_take(&data->bus_sem, K_FOREVER) != 0) return -1;

    uint32_t fifo_reg = (ESP_SDIO_REG_FIFO - len) & ESP_ADDRESS_MASK;

    /*Perform Block Transfer if possible */
    if(blocks > 0) {

	if(is_write) {

	    ret = sdio_write_blocks_fifo(func1, fifo_reg, buf, blocks);
	} else {

	    ret = sdio_read_blocks_fifo(func1, fifo_reg, buf, blocks);
	}
        if(ret) {

            LOG_ERR("SDIO Block Transfer failed: %d", ret);
            goto release_bus;
        }
    }
    if(remainder > 0) {
        /* Offset the pointer and the register address for the remaining bytes */
	uint8_t *rem_buf = buf + (blocks * block_size);
        ret = sdio_write_fifo(func1, fifo_reg, rem_buf, remainder);
    }

release_bus:
    k_sem_give(&data->bus_sem);
    return ret;

}
