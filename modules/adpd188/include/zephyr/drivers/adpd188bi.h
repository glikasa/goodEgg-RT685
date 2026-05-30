
#ifndef ZEPHYR_DRIVERS_SENSOR_ADPD188BI_H_
#define ZEPHYR_DRIVERS_SENSOR_ADPD188BI_H_

#include <zephyr/drivers/sensor.h>

enum smoke_channel {

    SMOKE_CHAN_BLUE = SENSOR_CHAN_PRIV_START,
    SMOKE_CHAN_IR,
};

#endif /* ZEPHYR_DRIVERS_SENSOR_ADPD188BI_H_ */
