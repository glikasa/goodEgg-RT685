
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adpd188bi.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/sys/util_macro.h>

void on_data_ready(const struct device *dev,
                   const struct sensor_trigger *trigger) {

  struct sensor_value smoke;
  if (sensor_channel_get(dev, SENSOR_CHAN_VOC, &smoke) < 0) {

    printk("%s: Failed to get smoke data \n", dev->name);
    return;
  }
  printk("%16s : Blue data: %d   IR data: %d\n", dev->name, smoke.val1,
         smoke.val2);
}

int main(void) {

  int ret;
  const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(adpd0));
  if (!device_is_ready(dev)) {

	printk("sensor: device %s not ready.\n", dev->name);
	return -EIO;
    }
	//    const struct sensor_trigger trig = {.type = SENSOR_TRIG_DATA_READY, .chan = ADPD188_CHAN_ALL};
	//    if(sensor_trigger_set(dev, &trig, on_data_ready) < 0) {
	//
	// printk("sensor: device %s could not set trigger.\n", dev->name);
	//    }
	//    printk("Smoke sensor started in interrupt mode");
	//    //set_sampling_freq(dev);
	//    while(1) {
	//
	// k_sleep(K_FOREVER);
	//    }
    struct sensor_value smoke_blue;
    struct sensor_value smoke_ir;
    while (1) {

	ret = sensor_sample_fetch(dev);
	if(ret < 0) {
	    printk("%s: sensor_sample_fetch() failed: %d\n", dev->name, ret);
	    return ret;
	}

	ret = sensor_channel_get(dev, SMOKE_CHAN_BLUE, &smoke_blue);
	ret = sensor_channel_get(dev, SMOKE_CHAN_IR, &smoke_ir);
	if (ret < 0) {
	    printk("%s: Failed to get smoke data %d\n", dev->name, ret);
	    return ret;
	}
	printk("%16s : Blue data: %d   IR data: %d\n", dev->name, smoke_blue.val1, smoke_ir.val1);
	k_msleep(1000);
    }
    return 0;
}
