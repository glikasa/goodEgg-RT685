
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util_macro.h>
#include <zephyr/kernel.h>
#include <zephyr/rtio/rtio.h>


int main(void) {

    int ret;
    const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(adpd0));
    if(!device_is_ready(dev)) {

	printk("sensor: device %s not ready.\n", dev->name);
	return -EIO;
    }
    //set_sampling_freq(dev);
    while (1) {

	struct sensor_value smoke;
	ret = sensor_sample_fetch(dev);
	if(ret < 0) {
	    printk("%s: sensor_sample_fetch() failed: %d\n", dev->name, ret);
	    return ret;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_VOC, &smoke);
	if (ret < 0) {
	    printk("%s: Failed to get smoke data %d\n", dev->name, ret);
	    return ret;
	}
	printk("%16s : Blue data: %d   IR data: %d\n", dev->name, smoke.val1, smoke.val2);
	k_msleep(1000);
    }
    return 0;
}
