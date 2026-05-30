/*
 * SPDX-License-Identifier: Apache-2.0
 * ESP Hosted SDIO HAL Implementation
 */

#include "zephyr/sd/sd.h"
#include <stdint.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sd/sdio.h>
#include <zephyr/sd/sd_spec.h>
#include <esp_hosted_hal_sdio.h>
#include <esp_hosted_wifi.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(esp_hosted_hal, CONFIG_WIFI_LOG_LEVEL);


#define ESP_SLAVE_SLCHOST_BASE         0//0x3FF55000
/* Scratch registers*/
#define ESP_SLAVE_SCRATCH_REG_0        (ESP_SLAVE_SLCHOST_BASE + 0x6C)
#define ESP_SLAVE_SCRATCH_REG_1        (ESP_SLAVE_SLCHOST_BASE + 0x70)
#define ESP_SLAVE_SCRATCH_REG_2        (ESP_SLAVE_SLCHOST_BASE + 0x74)
#define ESP_SLAVE_SCRATCH_REG_3        (ESP_SLAVE_SLCHOST_BASE + 0x78)
#define ESP_SLAVE_SCRATCH_REG_4        (ESP_SLAVE_SLCHOST_BASE + 0x7C)
#define ESP_SLAVE_SCRATCH_REG_6        (ESP_SLAVE_SLCHOST_BASE + 0x88)
#define ESP_SLAVE_SCRATCH_REG_7        (ESP_SLAVE_SLCHOST_BASE + 0x8C)
#define ESP_SLAVE_SCRATCH_REG_8        (ESP_SLAVE_SLCHOST_BASE + 0x9C)
#define ESP_SLAVE_SCRATCH_REG_9        (ESP_SLAVE_SLCHOST_BASE + 0xA0)
#define ESP_SLAVE_SCRATCH_REG_10       (ESP_SLAVE_SLCHOST_BASE + 0xA4)
#define ESP_SLAVE_SCRATCH_REG_11       (ESP_SLAVE_SLCHOST_BASE + 0xA8)
#define ESP_SLAVE_SCRATCH_REG_12       (ESP_SLAVE_SLCHOST_BASE + 0xAC)
#define ESP_SLAVE_SCRATCH_REG_13       (ESP_SLAVE_SLCHOST_BASE + 0xB0)
#define ESP_SLAVE_SCRATCH_REG_14       (ESP_SLAVE_SLCHOST_BASE + 0xB4)
#define ESP_SLAVE_SCRATCH_REG_15       (ESP_SLAVE_SLCHOST_BASE + 0xB8)

#define HOST_TO_SLAVE_INTR             ESP_SLAVE_SCRATCH_REG_7

/* SLAVE registers */
/* Interrupt Registers */
#define ESP_SLAVE_INT_RAW_REG          (ESP_SLAVE_SLCHOST_BASE + 0x50)
#define ESP_SLAVE_INT_ST_REG           (ESP_SLAVE_SLCHOST_BASE + 0x58)
#define ESP_SLAVE_INT_CLR_REG          (ESP_SLAVE_SLCHOST_BASE + 0xD4)
#define ESP_HOST_INT_ENA_REG           (ESP_SLAVE_SLCHOST_BASE + 0xDC)

/* Host side interrupts for ESP_HOST_INT_ENA_REG */
  #define SDIO_INT_NEW_PACKET          BIT(23)
  #define SDIO_INT_START_THROTTLE      BIT(7)
  #define SDIO_INT_STOP_THROTTLE       BIT(6)

/* Data path registers*/
#define ESP_SLAVE_PACKET_LEN_REG       (ESP_SLAVE_SLCHOST_BASE + 0x60)
#define ESP_SLAVE_TOKEN_RDATA          (ESP_SLAVE_SLCHOST_BASE + 0x44)
#define ESP_SDIO_REG_DEV_CAPABILITIES   ESP_SLAVE_SCRATCH_REG_0

#define ESP_ADDRESS_MASK               (0x3FF)
#define ESP_SDIO_REG_FIFO               0x1F800
#define ESP_ADDRESS_MASK                0x1FFFF

/* Host to Slave interrupt bits (0x3FF5508C) */
#define ESP_OPEN_DATA_PATH              BIT(0)
#define ESP_CLOSE_DATA_PATH             BIT(1)
#define ESP_RESET_SDIO_QUEUE            BIT(2)

/* Device capabilities bits (0x3FF5506C) */
#define ESP_CAP_WLAN_SUPPORT            BIT(0)

/* Buffer configuration */
#define ESP_RX_BUFFER_SIZE              2048
#define ESP_TX_BYTE_MODE_THRESHOLD      512
#define ESP_BLOCK_SIZE                  512


inline static int esp_write_reg32(struct sdio_func *func1, uint32_t reg, uint32_t value) {

    uint8_t data[4];
    data[0] = value & 0xFF;
    data[1] = (value >> 8) & 0xFF;
    data[2] = (value >> 16) & 0xFF;
    data[3] = (value >> 24) & 0xFF;
    return sdio_write_addr(func1, reg, data, 4);
}

/**
 * Read ESP slave register (32-bit)
 */
inline static int esp_read_reg32(struct sdio_func *func1, uint32_t addr, uint32_t *value) {

    static int count = 8;
    uint8_t data[4];
    int ret;

    ret = sdio_read_byte(func1, addr, &data[0]);
    ret = sdio_read_byte(func1, addr + 1, &data[1]);
    ret = sdio_read_byte(func1, addr + 2, &data[2]);
    ret = sdio_read_byte(func1, addr + 3, &data[3]);
    //ret = sdio_read_addr(func1, addr, data, 4);
    if(!ret) {

        *value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
        if(-- count > 0) {

        LOG_INF("Raw bytes: %02x %02x %02x %02x", 
                ((uint8_t*)&data)[0], ((uint8_t*)&data)[1], 
                ((uint8_t*)&data)[2], ((uint8_t*)&data)[3]);
        }
    }
    return ret;
}

/**
 * Write to ESP slave control register (interrupt register)
 */
static int esp_write_host_slave_intr_reg(struct sdio_func *func1, uint32_t value) {

    return esp_write_reg32(func1, HOST_TO_SLAVE_INTR, value);
}

static int esp_write_host_intr_enable_reg(struct sdio_func *func1, uint32_t value) {

    return esp_write_reg32(func1, ESP_HOST_INT_ENA_REG, value);
}

static int esp_write_slave_intr_clear_reg(struct sdio_func *func1, uint32_t value) {

    return esp_write_reg32(func1, ESP_SLAVE_INT_CLR_REG, value);
}

/**
 * Read receiving buffer count (TOKEN1 field, bits 27-16 of TOKEN_RDATA)
 */
static int esp_get_rx_buffer_count(struct sdio_func *func1, uint16_t *buf_count) {

    uint32_t token_reg;
    int ret;
    LOG_INF("Reading buffer count");
    ret = esp_read_reg32(func1, ESP_SLAVE_TOKEN_RDATA, &token_reg);
    if(!ret) {

        *buf_count = (token_reg >> 16) & 0xFFF; /* Extract bits 27-16 */
    }
    return ret;
}

/**
 * Read accumulated packet length from slave
 */
static int esp_get_packet_length(struct sdio_func *func1, uint32_t *pkt_len) {

    LOG_INF("Reading packet length");
    return esp_read_reg32(func1, ESP_SLAVE_PACKET_LEN_REG, pkt_len);
}

/**
 * Check interrupt status
 */
static int esp_get_interrupt_status(struct sdio_func *func1, uint32_t *int_st) {

    return esp_read_reg32(func1, ESP_SLAVE_INT_ST_REG, int_st);
}

/**
 * Get device capabilities
 */
static int esp_get_capabilities(struct sdio_func *func1, uint32_t *caps) {

    return esp_read_reg32(func1, ESP_SDIO_REG_DEV_CAPABILITIES, caps);
}


/**
 * @brief Check if RX data is available
 * @param dev Device instance
 * @param len Output: available data length
 * @return 0 if data available, -EAGAIN if not, other negative on error
 */
static int esp_sdio_data_ready(struct sdio_func *func1) {

    uint32_t int_reg;
    if(esp_get_interrupt_status(func1, &int_reg)) {

        return -EINVAL;
    }
    if(int_reg & SDIO_INT_NEW_PACKET) {

        if(esp_write_slave_intr_clear_reg(func1, int_reg)) {

            return -EIO;
        }
        return 0;
    }
    return -EAGAIN;
}


int esp_hosted_sdio_init(struct sdio_func *func1) {

    int ret;
    /* === ESP-Hosted Protocol Initialization === */
    /*Soft reset SDIO interface */
    LOG_INF("Resetting ESP SDIO queues...");
    ret = esp_write_host_slave_intr_reg(func1, ESP_RESET_SDIO_QUEUE);
    if(ret) {

        LOG_ERR("Failed to reset SDIO queues: %d", ret);
        return ret;
    }
    k_msleep(100);  /* Give ESP time to reset queues */
    /*
     * Read and store initial counters
     */
    uint32_t pkt_len;
    ret = esp_get_packet_length(func1, &pkt_len);
    if(ret) {

        LOG_ERR("Failed to read packet length: %d", ret);
        return ret;
    }
    LOG_INF("Initial packet length: %u", pkt_len);
    uint16_t buf_count;
    ret = esp_get_rx_buffer_count(func1, &buf_count);
    if (ret) {
        LOG_ERR("Failed to read buffer count: %d", ret);
        return ret;
    }
    LOG_INF("Initial buffer count: %u", buf_count);

    /* Open data path */
    LOG_INF("Opening data path...");
    ret = esp_write_host_slave_intr_reg(func1, ESP_OPEN_DATA_PATH);
    if(ret) {

        LOG_ERR("Failed to open data path: %d", ret);
        return ret;
    }
    //k_msleep(500);  /* Give ESP time to open data path */
    return 0;
}

int esp_hosted_hal_init(const struct device *dev) {

    const esp_hosted_config_t *config = dev->config;
    esp_hosted_data_t *data = dev->data;
    int ret;
    uint8_t reg_val;

    LOG_INF("Starting ESP SDIO initialization...");
    if(!device_is_ready(config->sdhc_dev)) {

        LOG_ERR("SDHC device is not ready");
        return -ENODEV;
    }

    /* Reset ESP32 slave */
    LOG_INF("Resetting ESP32 slave...");
    gpio_pin_configure_dt(&config->reset_gpio, GPIO_OUTPUT_INACTIVE);
    gpio_pin_set_dt(&config->reset_gpio, 1);  /* Assert reset */
    k_msleep(200);
    gpio_pin_set_dt(&config->reset_gpio, 0);  /* Deassert reset */
    k_msleep(5000);  /* Wait for ESP32 boot */

    /* Initialize SD/SDIO (includes CMD5, CMD3, CMD7) */
    ret = sd_init(config->sdhc_dev, &data->card);
    if(ret) {

        LOG_ERR("sd_init failed: %d", ret);
        return ret;
    }

    /* Initialize Function 0 */
    ret = sdio_init_func(&data->card, &data->card.func0, SDIO_FUNC_NUM_0);
    if(ret) {

        LOG_ERR("sdio_init_func func0 failed: %d", ret);
        return ret;
    }

    /* Initialize Function 1 */
    ret = sdio_init_func(&data->card, &data->sdio_func1, SDIO_FUNC_NUM_1);
    if(ret) {

        LOG_ERR("sdio_init_func func1 failed: %d", ret);
        return ret;
    }

    /* Enable Function 1 (CMD52 Write 0x02 = 0x02) */
    ret = sdio_enable_func(&data->sdio_func1);
    if(ret) {

        LOG_ERR("Failed to enable Function 1: %d", ret);
        return ret;
    }

    /* Set Func1 blocksize to 512 */
    ret = sdio_set_block_size(&data->sdio_func1, ESP_BLOCK_SIZE);
    if(ret) {
        LOG_ERR("Failed to set block size: %d", ret);
        return ret;
    }
    ret = esp_hosted_sdio_init(&data->sdio_func1);
    if(ret) {
        LOG_ERR("Failed to Initialize esp hosted sdio: %d", ret);
        return ret;
    }
    /* Initialize read length tracker */
    data->cumulative_read_len = 0;
    LOG_INF("ESP hosted SDIO initialized successfully");
    return 0;
}

/**
 * Write data to ESP32 slave (Receiving FIFO from slave's perspective)
 */
static int esp_sdio_write_fifo(const struct device *dev, 
                                const uint8_t *buf, 
                                size_t len) {
    esp_hosted_data_t *data = dev->data;
    struct sdio_func *func1 = &data->sdio_func1;
    uint16_t token_reg;
    uint16_t buf_available;
    uint32_t buffers_needed;
    int ret;

    LOG_INF("Writing length %u data", len);
    if(len > ESP_RX_BUFFER_SIZE) {

        LOG_ERR("Packet too large: %u (max %u)", len, ESP_RX_BUFFER_SIZE);
        return -EINVAL;
    }
    /*Read TOKEN1 field for available buffers */
    ret = esp_get_rx_buffer_count(func1, &token_reg);
    if(ret) {

        LOG_ERR("Failed to read token register: %d", ret);
        return ret;
    }

    LOG_INF("Available buffer length: %d", token_reg);
    if(data->cumulative_num_buffers_writtern > token_reg) {

        data->cumulative_num_buffers_writtern = 0;
    }
    buf_available = token_reg - data->cumulative_num_buffers_writtern;

    /*Check if sufficient buffers available */
    buffers_needed = (len + ESP_RX_BUFFER_SIZE - 1) / ESP_RX_BUFFER_SIZE;
    if(buf_available < buffers_needed) {

        LOG_WRN("Insufficient buffers: need %u, have %u", 
                buffers_needed, buf_available);
        return -EBUSY;
    }
    uint32_t fifo_addr = ESP_SDIO_REG_FIFO - len;
    // uint32_t blocks = len / ESP_BLOCK_SIZE;
    // uint32_t remainder = len % ESP_BLOCK_SIZE;
    ret = sdio_write_addr(func1, fifo_addr, (uint8_t*)buf, len);
    if(ret) {

        LOG_ERR("SDIO write failed: %d", ret);
        return ret;
    }
    // if(blocks > 0) {
    //
    //     /* Write to FIFO. Address = 0x1F800 - requested_length */
    //     // uint32_t blk_len = blocks * ESP_BLOCK_SIZE;
    //     // fifo_addr = ESP_SDIO_REG_FIFO - blk_len;
    //     ret = sdio_write_blocks_fifo(func1, fifo_addr, (uint8_t *)buf, blocks);
    //     if(ret) {
    //
    //         LOG_ERR("Block write failed: %d", ret);
    //         return ret;
    //     }
    // }
    // if(remainder > 0) {
    //
    //     /* For remainder, CMD53 byte mode with proper address */
    //     fifo_addr = ESP_SDIO_REG_FIFO - remainder;
    //     ret = sdio_write_addr(func1, fifo_addr, 
    //                          (uint8_t *)&buf[blocks * ESP_BLOCK_SIZE], 
    //                          remainder);
    //     if(ret) {
    //
    //         LOG_ERR("Remainder write failed: %d", ret);
    //         return ret;
    //     }
    // }
    data->cumulative_num_buffers_writtern += buffers_needed;
    LOG_DBG("Wrote %u bytes to slave", len);
    return 0;
}

/**
 * Read data from ESP32 slave (Sending FIFO from slave's perspective)
 */
static int esp_sdio_read_fifo(const struct device *dev, 
                               uint8_t *buf, 
                               size_t len) {
    esp_hosted_data_t *data = dev->data;
    struct sdio_func *func1 = &data->sdio_func1;
    uint32_t pkt_len_accumulated;
    uint32_t available_len;
    int ret;

    //uint32_t status_reg;
    // ret = esp_get_interrupt_status(func1, &status_reg);
    //
    // if(ret) {
    //
    //     LOG_ERR("Failed read Status register: %d", ret);
    //     return ret;
    // }
    // if(!(status_reg & SDIO_INT_NEW_PACKET)) {
    //
    //     return -EAGAIN;
    // }
    /* Read PKT_LEN register */
    ret = esp_get_packet_length(func1, &pkt_len_accumulated);
    if(ret) {

        return ret;
    }

    LOG_INF("Total Available and read packet sofar: %u : %u", pkt_len_accumulated, data->cumulative_read_len);

    /* Calculate available data length */
    available_len = pkt_len_accumulated - data->cumulative_read_len;
    if(available_len == 0) {

        return -EAGAIN;  /* No data available */
    }
    if(available_len < len) {

        LOG_WRN("Requested %u bytes, only %u available", len, available_len);
        len = available_len;
    }
    uint32_t fifo_addr = ESP_SDIO_REG_FIFO - len;
    // uint32_t blocks = len / ESP_BLOCK_SIZE;
    // uint32_t remainder = len % ESP_BLOCK_SIZE;

    ret = sdio_read_addr(func1, fifo_addr, buf, len);
    if(ret) {

        LOG_ERR("SDIO read failed: %d", ret);
        return ret;
    }

    // for(int i = 0; i < 200; i ++) {
    //
    //     LOG_INF("%d", buf[i]);
    // }

    // if(blocks > 0) {
    //
    //     /* Read from FIFO. Address = 0x1F800 - requested_length */
    //     // uint32_t blk_len = blocks * ESP_BLOCK_SIZE;
    //     // fifo_addr = ESP_SDIO_REG_FIFO - blk_len;
    //     ret = sdio_read_blocks_fifo(func1, fifo_addr, buf, blocks);
    //     if(ret) {
    //
    //         LOG_ERR("Block read failed: %d", ret);
    //         return ret;
    //     }
    // }
    //
    // if(remainder > 0) {
    //
    //     fifo_addr = ESP_SDIO_REG_FIFO - remainder;
    //     ret = sdio_read_addr(func1, fifo_addr, 
    //                         &buf[blocks * ESP_BLOCK_SIZE], 
    //                         remainder);
    //     if(ret) {
    //
    //         LOG_ERR("Remainder read failed: %d", ret);
    //         return ret;
    //     }
    // }
    // if(esp_write_slave_intr_clear_reg(func1, status_reg)) {
    //
    //     LOG_ERR("Failed clear Status register: %d", ret);
    //     return -EIO;
    // }
    data->cumulative_read_len += len;

    /* Clear interrupt if packet fully consumed */
    if (data->cumulative_read_len >= pkt_len_accumulated) {
        //esp_clear_interrupts(func1, int_st);
        data->cumulative_read_len = 0;  /* Reset for next packet */
    }
    //LOG_INF("Read %u bytes from slave", len);
    return len;  /* Return actual bytes read */
}

/**
 * Combined transfer function (maintains your signature)
 */
int esp_hosted_hal_sdio_transfer(const struct device *dev,
                                  esp_frame_t *tx_frame,
                                  esp_frame_t *rx_frame,
                                  size_t len) {
    int ret = 0;
    esp_hosted_data_t *data = dev->data;

    if(k_sem_take(&data->bus_sem, K_MSEC(1000)) != 0) {

        LOG_ERR("Bus semaphore timeout");
        return -ETIMEDOUT;
    }
    if(tx_frame != NULL) {

        ret = esp_sdio_write_fifo(dev, (uint8_t *)tx_frame, len);
        if(ret) {

            LOG_ERR("SDIO write failed: %d", ret);
            goto release;
        }
    }
    if(rx_frame != NULL) {

        ret = esp_sdio_read_fifo(dev, (uint8_t *)rx_frame, len);
        if(ret > 0) {

            ret = 0;
        }
        if(ret < 0 && ret != -EAGAIN) {

            LOG_ERR("SDIO read failed: %d", ret);
        }
    }
release:
    k_sem_give(&data->bus_sem);
    return ret;
}


