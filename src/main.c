/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zephyr/dt-bindings/adc/adc.h"
#include <stdio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

/* ADC node from the devicetree. */
#define ADC_NODE DT_ALIAS(adc0)

/* Auxiliary macro to obtain channel vref, if available. */
#define CHANNEL_VREF(node_id) DT_PROP_OR(node_id, zephyr_vref_mv, 0)

/* Data of ADC device specified in devicetree. */
static const struct device *adc = DEVICE_DT_GET(ADC_NODE);

/* Common initialization for all channels */
static struct adc_channel_cfg channel_cfgs[2] = {
    {
        .channel_id = 0,
        .gain = ADC_GAIN_1_2,
        .reference = ADC_REF_EXTERNAL0,
        .differential = 0,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 100),

    },
    {
        .channel_id = 8,
        .gain = ADC_GAIN_1_2,
        .reference = ADC_REF_EXTERNAL0,
        .differential = 0,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 100), //ADC_ACQ_TIME_DEFAULT,
    }};

/* Data array of ADC channels for the specified ADC. */
// static const struct adc_channel_cfg channel_cfgs[] = {
// 	DT_FOREACH_CHILD_SEP(ADC_NODE, ADC_CHANNEL_CFG_DT, (,))};
//
// /* Data array of ADC channel voltage references. */
// static uint32_t vrefs_mv[] = {DT_FOREACH_CHILD_SEP(ADC_NODE, CHANNEL_VREF,
// (,))};

static uint32_t vrefs_mv[] = {1800, 1800};

/* Get the number of channels defined on the DTS. */
#define CHANNEL_COUNT ARRAY_SIZE(channel_cfgs)

int main(void) {
  int err;
  uint32_t count = 0;
  uint16_t channel_reading[CONFIG_SEQUENCE_SAMPLES][CHANNEL_COUNT];

  /* Options for the sequence sampling. */
  const struct adc_sequence_options options = {
      .extra_samplings = CONFIG_SEQUENCE_SAMPLES - 1,
      .interval_us = 0,
  };

  /* Configure the sampling sequence to be made. */
  struct adc_sequence sequence = {
      .buffer = channel_reading,
      /* buffer size in bytes, not number of samples */
      .buffer_size = sizeof(channel_reading),
      .resolution = CONFIG_SEQUENCE_RESOLUTION,
      .oversampling = CONFIG_SEQUENCE_OVERSAMPLING,
      .options = &options,
  };

  if (!device_is_ready(adc)) {
    printf("ADC controller device %s not ready\n", adc->name);
    return 0;
  }

  /* Configure channels individually prior to sampling. */
  for (int i = 0; i < CHANNEL_COUNT; i++) {

    sequence.channels |= BIT(channel_cfgs[i].channel_id);
    err = adc_channel_setup(adc, &channel_cfgs[i]);
    if (err < 0) {
      printf("Could not setup channel #%d (%d)\n", i, err);
      return 0;
    }
    if ((vrefs_mv[i] == 0) && (channel_cfgs[i].reference == ADC_REF_INTERNAL)) {
      vrefs_mv[i] = adc_ref_internal(adc);
    }
  }

  while (1) {
    printf("ADC sequence reading [%u]:\n", count++);
    k_msleep(2000);

    err = adc_read(adc, &sequence);
    if (err < 0) {
      printf("Could not read (%d)\n", err);
      continue;
    }

    for (int channel_index = 0; channel_index < CHANNEL_COUNT;
         channel_index++) {
      int32_t val_mv;

      printf("- %s, channel %" PRId32 ", %" PRId32 " sequence samples:\n",
             adc->name, channel_cfgs[channel_index].channel_id,
             CONFIG_SEQUENCE_SAMPLES);
      for (int sample_index = 0; sample_index < CONFIG_SEQUENCE_SAMPLES;
           sample_index++) {

        uint8_t res = CONFIG_SEQUENCE_RESOLUTION;

        /*
         * If using differential mode, the 16/32 bit value
         * in the ADC sample buffer should be a signed 2's
         * complement value.
         * Also reduce the resolution by 1 for the conversion
         */
        if (channel_cfgs[channel_index].differential) {
          val_mv =
              (int32_t)((int16_t)channel_reading[sample_index][channel_index]);
          res -= 1;
        } else {
          val_mv = channel_reading[sample_index][channel_index];
        }
        printf("- - %" PRId32, val_mv);
        err = adc_raw_to_millivolts(vrefs_mv[channel_index],
                                    channel_cfgs[channel_index].gain, res,
                                    &val_mv);

        /* conversion to mV may not be supported, skip if not */
        if ((err < 0) || vrefs_mv[channel_index] == 0) {
          printf(" (value in mV not available)\n");
        } else {
          printf(" = %" PRId32 "mV\n", val_mv);
        }
      }
    }
  }

  return 0;
}

// /* 1000 msec = 1 sec */
// #define SLEEP_TIME_MS   1000
//
// /* The devicetree node identifier for the "led0" alias. */
// #define LED0_NODE DT_ALIAS(led0)
//
// /*
//  * A build error on this line means your board is unsupported.
//  * See the sample documentation for information on how to fix this.
//  */
// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
//
// int main(void) {
//
// 	int ret;
// 	// bool led_state = true;
// 	//
// 	if(!gpio_is_ready_dt(&led)) {
//
// 		return 0;
// 	}
// 	//
// 	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
// 	if (ret < 0) {
// 		return 0;
// 	}
//
// 	while (1) {
//
// 		ret = gpio_pin_toggle_dt(&led);
// 		if (ret < 0) {
// 			return 0;
// 		}
// 		//led_state = !led_state;
// 		printf("Hello world from GoodEGG board!\n");
// 		k_msleep(SLEEP_TIME_MS);
// 	}
// 	return 0;
// }
