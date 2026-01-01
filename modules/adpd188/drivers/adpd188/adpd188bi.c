#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <stdint.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include "no_os_i2c.h"
#include "adpd188.h"

#define DT_DRV_COMPAT analog_adpd188

LOG_MODULE_REGISTER(adpd188);

struct adpd188bi_config {
  
    struct i2c_dt_spec i2c;

    uint32_t i2c_clock_freq;
};

typedef struct {

  int32_t blue_data;

  int32_t ir_data;

}smoke_data_t;

struct adpd188bi_data {

  struct adpd188_dev *adpd_dev;

  smoke_data_t smoke_data;

};

static struct adpd188_dev adpd188_dev;

void *no_os_calloc(size_t nitems, size_t size) {

    return &adpd188_dev;
}

void no_os_free(void *ptr) {}

void no_os_udelay(uint32_t usecs) {

  k_busy_wait(usecs);
}

void no_os_mdelay(uint32_t msecs) {

  k_msleep(msecs);
}

/* Initialize the I2C communication peripheral. */
int32_t no_os_i2c_init(struct no_os_i2c_desc **desc,
		       const struct no_os_i2c_init_param *param) {

    static struct no_os_i2c_desc static_desc;

    static_desc.device_id = param->device_id;
    static_desc.max_speed_hz = param->max_speed_hz;
    static_desc.platform_ops = param->platform_ops;
    static_desc.slave_address = param->slave_address;
    static_desc.extra = param->extra;

    *desc = &static_desc;
    return 0;
}

/* Free the resources allocated by no_os_i2c_init(). */
int32_t no_os_i2c_remove(struct no_os_i2c_desc *desc) {return 0;}

/* Write data to a slave device. */
int32_t no_os_i2c_write(struct no_os_i2c_desc *desc,
			uint8_t *data,
			uint8_t bytes_number,
			uint8_t stop_bit) {

    const struct device *i2c_dev = (const struct device *)desc->extra;

    struct i2c_msg msg = {
	  .buf = data,
	  .len = bytes_number,
	  .flags = I2C_MSG_WRITE,
    };

    /* If stop_bit == 0, suppress STOP */
    if(!stop_bit) {
	msg.flags |= I2C_MSG_STOP;   /* STOP is default */
	msg.flags &= ~I2C_MSG_STOP;  /* explicitly remove STOP */
    }
    int ret = i2c_transfer(i2c_dev, &msg, 1, desc->slave_address);
    if(ret != 0) {

	LOG_ERR("Failed to write i2c: address: %02x, err: %d", desc->slave_address, ret);
    }
    return ret == 0 ? 0 : -EIO;
}

/* Read data from a slave device. */
int32_t no_os_i2c_read(struct no_os_i2c_desc *desc,
		       uint8_t *data,
		       uint8_t bytes_number,
		       uint8_t stop_bit) {

    const struct device *i2c_dev = (const struct device *)desc->extra;
    struct i2c_msg msg = {
	  .buf = data,
	  .len = bytes_number,
	  .flags = I2C_MSG_READ,
    };

    /* If stop_bit == 0, suppress STOP */
    if (!stop_bit) {
      msg.flags |= I2C_MSG_STOP;
      msg.flags &= ~I2C_MSG_STOP;
    }
    int ret = i2c_transfer(i2c_dev,&msg, 1, desc->slave_address);
    if(ret != 0) {

	LOG_ERR("Failed to read i2c: address %02x, err: %d", desc->slave_address, ret);
    }
    return ret == 0 ? 0 : -EIO;
}

/* Initialize I2C bus descriptor*/
int32_t no_os_i2cbus_init(const struct no_os_i2c_init_param *param) {return 0;}

/* Free the resources allocated for I2C  bus desc*/
void no_os_i2cbus_remove(uint32_t bus_number) {}


int32_t no_os_spi_init(struct no_os_spi_desc **spi_desc, const struct no_os_spi_init_param * params) {

    return 0;
}


int32_t no_os_spi_remove(struct no_os_spi_desc *spi_desc) {

    return 0;
}

int32_t no_os_spi_write_and_read(struct no_os_spi_desc *desc, uint8_t *data, uint16_t bytes_number) {

    return 0;
}

/* Iterate over the spi_msg array and send all messages at once */
int32_t no_os_spi_transfer(struct no_os_spi_desc *desc, struct no_os_spi_msg *msgs, uint32_t len) {

    return 0;
}

/* Transfer a list of messages using DMA. Wait until all transfers are done */
int32_t no_os_spi_transfer_dma(struct no_os_spi_desc *desc, struct no_os_spi_msg *msgs, uint32_t len) {

   return 0;
}

/*
 * Transfer a list of messages using DMA. Return once the first one started and
 * invoke a callback when they are done.
 */
int32_t no_os_spi_transfer_dma_async(struct no_os_spi_desc *desc,
				     struct no_os_spi_msg *msgs,
				     uint32_t len,
				     void (*callback)(void *),
				     void *ctx){

    return 0;

}

/* Abort SPI transfers. */
int32_t no_os_spi_transfer_abort(struct no_os_spi_desc *desc) {

    return 0;
}


int32_t no_os_gpio_get(struct no_os_gpio_desc **desc,
		       const struct no_os_gpio_init_param *param){return 0;}

/* Obtain optional GPIO descriptor. */
int32_t no_os_gpio_get_optional(struct no_os_gpio_desc **desc,
				const struct no_os_gpio_init_param *param){return 0;}

/* Free the resources allocated by no_os_gpio_get(). */
int32_t no_os_gpio_remove(struct no_os_gpio_desc *desc){return 0;}

/* Enable the input direction of the specified GPIO. */
int32_t no_os_gpio_direction_input(struct no_os_gpio_desc *desc){return 0;}

/* Enable the output direction of the specified GPIO. */
int32_t no_os_gpio_direction_output(struct no_os_gpio_desc *desc,
				    uint8_t value){return 0;}

/* Get the direction of the specified GPIO. */
int32_t no_os_gpio_get_direction(struct no_os_gpio_desc *desc,
				 uint8_t *direction){return 0;}

/* Set the value of the specified GPIO. */
int32_t no_os_gpio_set_value(struct no_os_gpio_desc *desc,
			     uint8_t value){return 0;}

/* Get the value of the specified GPIO. */
int32_t no_os_gpio_get_value(struct no_os_gpio_desc *desc,
			     uint8_t *value){return 0;}
static const struct no_os_i2c_platform_ops zephyr_i2c_ops = {
    // Pointers for init/remove are NULL since we are using static allocation
    .i2c_ops_init = no_os_i2c_init,
    .i2c_ops_write = no_os_i2c_write,
    .i2c_ops_read = no_os_i2c_read,
    .i2c_ops_remove = no_os_i2c_remove
};

static int adpd188bi_core_init(struct adpd188bi_data *data, const struct adpd188bi_config *config) {

    struct adpd188_init_param adpd_init;

    adpd_init.phy_opt = ADPD188_I2C;

    adpd_init.phy_init.i2c_phy.extra = (void*)config->i2c.bus;
    adpd_init.phy_init.i2c_phy.device_id = ADPD188BI;
    adpd_init.phy_init.i2c_phy.max_speed_hz = config->i2c_clock_freq;
    adpd_init.phy_init.i2c_phy.slave_address = config->i2c.addr;
    adpd_init.phy_init.i2c_phy.platform_ops = &zephyr_i2c_ops;


    /* GPIO initialization is generally dependent on the platform used */
    adpd_init.gpio0_init.number = 1; /* Platform dependent; this is a dummy value */
    adpd_init.gpio0_init.extra = NULL; /* Platform dependent; this is a dummy value */
    adpd_init.gpio1_init.number = 2; /* Platform dependent; this is a dummy value */
    adpd_init.gpio1_init.extra = NULL; /* Platform dependent; this is a dummy value */


    uint16_t reg_data;
    if(adpd188_init(&data->adpd_dev, &adpd_init) != 0) {

	LOG_ERR("Failed to initialze ADPD188");
	return -EIO;
    }
    /* Read device ID. For ADPD188BI it must be 0x16. */
    if(adpd188_reg_read(data->adpd_dev, ADPD188_REG_DEVID, &reg_data) != 0) {

	return -EIO;
    }
    if((reg_data & ADPD188_DEVID_DEV_ID_MASK) != 0x16) {

	LOG_ERR("Wrong device ID %d", reg_data & ADPD188_DEVID_DEV_ID_MASK);
	return -EIO;
    }
 
    /** Enable 32kHz clock */
    if(adpd188_reg_read(data->adpd_dev, ADPD188_REG_SAMPLE_CLK, &reg_data) != 0) {

	LOG_ERR("Failed to Enable the 32kHz clock");
	return -EIO;
    }
    reg_data |= ADPD188_SAMPLE_CLK_CLK32K_EN_MASK;
    if(adpd188_reg_write(data->adpd_dev, ADPD188_REG_SAMPLE_CLK, reg_data) != 0) {

	return -EIO;
    }
    /* Activate program mode */
    if(adpd188_mode_set(data->adpd_dev, ADPD188_PROGRAM) != 0) {

	LOG_ERR("Failed to set the device in program mode");
	return -EIO;
    }
    /* Initialize device in the datasheet smoke detection configuration */
    if(adpd188_smoke_detect_setup(data->adpd_dev) != 0) {

	LOG_ERR("Failed to set the device in program mode");
	return -EIO;
    }

    //Start Normal Mode operation : Enter Normal Sampling Mode (Register 0x10 = 0x0002) to begin operation.
    if(adpd188_mode_set(data->adpd_dev, ADPD188_NORMAL) != 0) {

	LOG_ERR("Failed to set the device in Normal Mode");
	return -EIO;
    }
    return 0;
}

static int adpd188bi_init(const struct device *dev){

    const struct adpd188bi_config *config = dev->config;
    struct adpd188bi_data *data = dev->data;
    
    if(!device_is_ready(config->i2c.bus)) {

	LOG_ERR("I2C bus not ready");
        return -ENODEV;
    }
    if(adpd188bi_core_init(data, config) != 0) {

        return -EIO;
    }
    return 0;
}


#define FULL_PACKET_SIZE       8

#define NUM_ACTIVE_TIMESLOTS   2 

//Sensor Sample Fetch (The core data acquisition)
static int adpd188bi_sample_fetch(const struct device *dev, enum sensor_channel chan) {

	//    if(chan != SENSOR_CHAN_ALL) {
	//
	// return -ENOTSUP;
	//    }
    (void)chan;
    struct adpd188bi_data *data = dev->data;
    uint8_t byte_no;
    uint16_t buff[(2 * NUM_ACTIVE_TIMESLOTS)];

    if(adpd188_fifo_status_get(data->adpd_dev, &byte_no) != 0) {

	return -EIO;
    }
    if(byte_no < FULL_PACKET_SIZE) {

	return 0;
    }

    for(int i = 0; i < (2 * NUM_ACTIVE_TIMESLOTS); i++) {

	if(adpd188_reg_read(data->adpd_dev, ADPD188_REG_FIFO_ACCESS, (buff + i)) != 0) {
	    return -EIO;
	}
    }
    data->smoke_data.blue_data = buff[0] | (buff[1] << 16);
    data->smoke_data.ir_data = buff[2] | (buff[3] << 16);
    return 0;
}

//Sensor Value Get (Retrieves fetched data)
static int adpd188bi_channel_get(const struct device *dev,
                                 enum sensor_channel chan,
                                 struct sensor_value *val)
{
   
	//    if(chan != SENSOR_CHAN_ALL) {
	//
	// return -ENOTSUP;
	//    }
    (void)chan;
    struct adpd188bi_data *data = dev->data;
    val->val1 = data->smoke_data.blue_data;
    val->val2 = data->smoke_data.ir_data;
    return 0;
}

// Maps the Zephyr sensor API calls to your implemented functions.
static const struct sensor_driver_api adpd188bi_api = {
    .sample_fetch = adpd188bi_sample_fetch,
    .channel_get = adpd188bi_channel_get,
};


//Zephyr Device Tree Macro for Instantiation
#define ADPD188BI_DEFINE(i)                                              \
    static struct adpd188bi_data adpd188bi_data_##i;                     \
    static const struct adpd188bi_config adpd188bi_config_##i = {        \
        .i2c = I2C_DT_SPEC_INST_GET(i),                                  \
        .i2c_clock_freq = 100000,                                        \
    };                                                                   \
                                                                         \
    DEVICE_DT_INST_DEFINE(i,                                             \
                          adpd188bi_init,                                \
                          NULL,                                          \
                          &adpd188bi_data_##i,                           \
                          &adpd188bi_config_##i,                         \
                          POST_KERNEL,                                   \
                          CONFIG_SENSOR_INIT_PRIORITY,                   \
                          &adpd188bi_api);

// Call the macro for the instance defined in the Device Tree
ADPD188BI_DEFINE(0)
