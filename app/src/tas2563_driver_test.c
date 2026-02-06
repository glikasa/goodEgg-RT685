
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util_macro.h>
#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/rtio/rtio.h>
#include <zephyr/drivers/tas2563_driver.h>
#include "sine_data.h"


#define TAS_NODE DT_NODELABEL(tas2563_amp)

int main(void) {

    const struct device *const dev = DEVICE_DT_GET(TAS_NODE);
    if(!device_is_ready(dev)) {

	printk("Speaker: device %s not ready.\n", dev->name);
	return -EIO;
    }
    printk("Turning on speaker...\n");
    tas_speaker_on(dev, 0); 

    tas_stream_audio(dev, SINE_3KHZ_44100, 176400);

    printk("Turning off speaker...\n");
    tas_speaker_off(dev);
    while(1) {

	k_sleep(K_FOREVER);
    }
    return 0;
}
