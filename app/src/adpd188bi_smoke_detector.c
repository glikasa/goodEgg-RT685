#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

// Assuming the no-os structures are available via this header
#include "no_os_i2c.h"
// Assuming the ADPD188BI core driver init function:
int32_t adpd188bi_core_init(struct no_os_i2c_desc *i2c_desc);

// --- Zephyr Driver Configuration (Passed via Device Tree) ---
struct adpd188bi_config {
  // Stores the I2C bus and address, ready for Zephyr I2C API calls
  struct i2c_dt_spec i2c;
};

// --- Zephyr Driver Runtime Data ---
struct adpd188bi_data {
  // Data structures for sensor readings, etc.
};

// --- 2. Zephyr Wrapper Functions (Write and Read) ---

// Write wrapper (similar to previous, using 'extra' for the I2C bus handle)
static int32_t zephyr_i2c_ops_write(struct no_os_i2c_desc *desc, uint8_t *data,
                                    uint8_t reg_addr, uint8_t len) {
  // 1. Retrieve the Zephyr I2C bus device pointer from the 'extra' field
  const struct device *i2c_dev = (const struct device *)desc->extra;

  // 2. Prepare the Zephyr transmit buffer (Reg Address + Data)
  uint8_t tx_buf[1 + len];
  tx_buf[0] = reg_addr;
  memcpy(&tx_buf[1], data, len);

  // 3. Call Zephyr I2C API
  return i2c_write(i2c_dev, tx_buf, 1 + len, desc->slave_address);
}

// Read wrapper (similar to previous, using 'extra' for the I2C bus handle)
static int32_t zephyr_i2c_ops_read(struct no_os_i2c_desc *desc, uint8_t *data,
                                   uint8_t reg_addr, uint8_t len) {
  // 1. Retrieve the Zephyr I2C bus device pointer
  const struct device *i2c_dev = (const struct device *)desc->extra;

  // 2. Call Zephyr I2C API (Write Register Address, then Read Data)
  return i2c_write_read(i2c_dev, desc->slave_address, &reg_addr, 1, data, len);
}

// --- 3. Zephyr's Platform Operations (No Init/Remove) ---
static const struct no_os_i2c_platform_ops zephyr_i2c_ops = {
    // Pointers for init/remove are NULL since we are using static allocation
    .i2c_ops_init = NULL,
    .i2c_ops_write = zephyr_i2c_ops_write,
    .i2c_ops_read = zephyr_i2c_ops_read,
    .i2c_ops_remove = NULL,
};
