/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// #include <stdio.h>
// #include <zephyr/kernel.h>
// #include <zephyr/device.h>
// #include <zephyr/drivers/sensor.h>
// #include <zephyr/sys/util_macro.h>
// #include <zephyr/kernel.h>
// #include <zephyr/rtio/rtio.h>
//
// #define ACCEL_ALIAS(i) DT_ALIAS(_CONCAT(accel, i))
// #define ACCELEROMETER_DEVICE(i, _)                                                           \
// 	IF_ENABLED(DT_NODE_EXISTS(ACCEL_ALIAS(i)), (DEVICE_DT_GET(ACCEL_ALIAS(i)),))
// #define NUM_SENSORS 1
//
// /* support up to 10 accelerometer sensors */
// static const struct device *const sensors[] = {LISTIFY(10, ACCELEROMETER_DEVICE, ())};
//
// static const enum sensor_channel channels[] = {
// 	SENSOR_CHAN_ACCEL_X,
// 	SENSOR_CHAN_ACCEL_Y,
// 	SENSOR_CHAN_ACCEL_Z,
// };
//
// static int print_accels(const struct device *dev){
//
//     int ret;
//     struct sensor_value accel[3];
//     ret = sensor_sample_fetch(dev);
//     if(ret < 0) {
// 	printk("%s: sensor_sample_fetch() failed: %d\n", dev->name, ret);
// 	return ret;
//     }
//
//     for (size_t i = 0; i < ARRAY_SIZE(channels); i++) {
// 	ret = sensor_channel_get(dev, channels[i], &accel[i]);
// 	if (ret < 0) {
// 	    printk("%s: sensor_channel_get(%c) failed: %d\n", dev->name, 'X' + i, ret);
// 	    return ret;
// 	}
//     }
//
//     printk("%16s [m/s^2]:    (%12.6f, %12.6f, %12.6f)\n", dev->name,
// 	   sensor_value_to_double(&accel[0]), sensor_value_to_double(&accel[1]),
// 	   sensor_value_to_double(&accel[2]));
//
//     return 0;
// }
//
// static int set_sampling_freq(const struct device *dev){
//
//     int ret;
//     struct sensor_value odr;
//
//     ret = sensor_attr_get(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);
//
//     /* If we don't get a frequency > 0, we set one */
//     if (ret != 0 || (odr.val1 == 0 && odr.val2 == 0)) {
// 	odr.val1 = 100;
// 	odr.val2 = 0;
//
// 	ret = sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY,
// 			      &odr);
//
// 	if(ret != 0) {
// 	    printk("%s : failed to set sampling frequency\n", dev->name);
// 	}
//     }
//     return 0;
// }
//
// int main(void) {
//
//     int ret;
//     const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(accel0));
//     if(!device_is_ready(dev)) {
//
// 	printk("sensor: device %s not ready.\n", dev->name);
// 	return 0;
//     }
//     set_sampling_freq(dev);
//     while (1) {
//
// 	ret = print_accels(dev);
// 	if(ret < 0) {
// 	    return 0;
// 	}
// 	k_msleep(1000);
//     }
//     return 0;
// }
//
//

#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);

int main(void) {

	int ret;
	// bool led_state = true;
	//
	if(!gpio_is_ready_dt(&led0)) {

		return 0;
	}
	//
	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	while (1) {

		ret = gpio_pin_toggle_dt(&led0);
		ret = gpio_pin_toggle_dt(&led1);
		if (ret < 0) {
			return 0;
		}
		//led_state = !led_state;
		printf("Hello world from GoodEGG board!\n");
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
