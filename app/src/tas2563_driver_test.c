
#include <stdint.h>
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
#include <zephyr/drivers/uart.h>

#define TAS_NODE DT_NODELABEL(tas2563_amp)
/* 1. Get the device tree node identifier for the 'uart0' alias */
#define UART_NODE DT_ALIAS(uart0)

/* 2. Get the device pointer */
static const struct device *const uart_dev = DEVICE_DT_GET(UART_NODE);
//#include <fsl_inputmux.h>
int main(void) {


	  printk("Turning on speaker...\n");
	   if (!device_is_ready(uart_dev)) {

	       printk("uart device not ready...\n");
	       return 0;
	   }

	   for (int i = 0; i < 2048; i++) {
	       /* This is the closest functional equivalent to a simple 'write' */
	       uart_poll_out(uart_dev, 0xAA);
	   }

	//    const struct device *const dev = DEVICE_DT_GET(TAS_NODE);
	//    if(!device_is_ready(dev)) {
	//
	// printk("Speaker: device %s not ready.\n", dev->name);
	// return -EIO;
	//    }
    // const struct tas2563_api_t *api = (const struct tas2563_api_t *)dev->api;
    // if(api && api->speaker_on) {
    //     printk("Turning on speaker...\n");
    //     // Using '1' for MUSIC profile or '2' for CALIBRATION as defined in your driver
    //     api->speaker_on(dev, 1); 
    // }
/* Use the SDK provided base pointer but cast to a uint32_t array */
//volatile uint32_t *mux_regs = (volatile uint32_t *)INPUTMUX;

/* In RT685, the DMA0 INMUX registers start at a specific offset. 
   Check your fsl_inputmux.h or the manual, but usually, it's: */
    // uint32_t fc3_tx_val = mux_regs[7]; 
    // uint32_t fc7_tx_val = mux_regs[15];
    //
    // printk("FC3 Mux: 0x%x, FC7 Mux: 0x%x\n", fc3_tx_val, fc7_tx_val);
   // tas_speaker_on(dev, 0); 

    //tas_stream_audio(dev, SINE_3KHZ_44100, 176400);

    printk("Turning off speaker...\n");
    //tas_speaker_off(dev);
    while(1) {

	k_sleep(K_FOREVER);
    }
    return 0;
}
