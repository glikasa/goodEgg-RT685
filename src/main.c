/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// #include "zephyr/dt-bindings/adc/adc.h"
// #include <stdio.h>
// #include <zephyr/drivers/adc.h>
// #include <zephyr/drivers/gpio.h>
// #include <zephyr/drivers/sensor.h>
// #include <zephyr/kernel.h>
//
// // 1. Define the node identifier for our ADC readings consumer.
// #define ADC_READINGS_NODE DT_PATH(adc_readings)
//
// // 2. Get the ADC controller device handle from the first channel definition.
// // We use the first channel's controller because all channels use the same
// // controller.
// #define ADC_DEV_ID DT_IO_CHANNELS_CTLR(DT_CHILD(ADC_READINGS_NODE, io_channels))
// // static const struct device *adc_dev = DEVICE_DT_GET(ADC_DEV_ID);
//
// // 3. Get the total number of channels defined in the io-channels property.
// // #define CHANNEL_COUNT DT_NUM_IO_CHANNELS(ADC_READINGS_NODE)
//
// // --- Configuration Arrays (Now filled by DTS macros) ---
//
// // Define a macro to extract the hardware channel number (the '0' or '2' from
// // the DTS)
// #define GET_HW_CHANNEL(idx) DT_IO_CHANNELS_INPUT_BY_IDX(ADC_READINGS_NODE, idx)
//
// /* ADC node from the devicetree. */
// #define ADC_NODE DT_ALIAS(adc0)
//
// /* Auxiliary macro to obtain channel vref, if available. */
// #define CHANNEL_VREF(node_id) DT_PROP_OR(node_id, zephyr_vref_mv, 0)
//
// /* ADC device specified in devicetree. */
// static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);
//
// // Define a macro to extract the gain, acquisition time, etc., using the
// // DT-defined channel index. NOTE: Since the properties (gain, acquisition-time,
// // etc.) are NOT defined in the child nodes (because you are using io-channels),
// // these values must be hardcoded or defined on the 'readings' node itself, but
// // this is less common. We will use hardcoded values for simplicity but ensure
// // the channel ID mapping is dynamic.
//
// static struct adc_channel_cfg channel_cfgs[] = {
//     {
//         .channel_id = 0,
//         .gain = ADC_GAIN_1_2,
//         .reference = ADC_REF_EXTERNAL0,
//         .differential = 0,
//         .acquisition_time =
//             ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131), // ADC_ACQ_TIME_DEFAULT,
//         .input_positive = 0,
//     },
//     {
//         .channel_id = 2,
//         .gain = ADC_GAIN_1_2,
//         .reference = ADC_REF_EXTERNAL0,
//         .differential = 0,
//         .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
//         .input_positive = 2,
//         // ADC_ACQ_TIME_DEFAULT,
//
//     },
//     //   {
//     // .channel_id = 10,
//     //       .gain = ADC_GAIN_1_2,
//     //       .reference = ADC_REF_EXTERNAL0,
//     // .differential = 0,
//     //       .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
//     //       //ADC_ACQ_TIME_DEFAULT,
//     //   },
//     //   {
//     // .channel_id = 11,
//     //       .gain = ADC_GAIN_1_2,
//     //       .reference = ADC_REF_EXTERNAL0,
//     // .differential = 0,
//     //       .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
//     //       //ADC_ACQ_TIME_DEFAULT,
//     //   },
//
// };
//
// #define CHANNEL_COUNT ARRAY_SIZE(channel_cfgs)
// // Data array of ADC channel voltage references (mV) (Hardcoded for this
// // example)
// static const uint32_t VREFS_MV[CHANNEL_COUNT] = {1800, 1800};
//
// // Buffer size for a single channel read (accounting for oversampling)
// #define BUFFER_SIZE CHANNEL_COUNT //(CONFIG_SEQUENCE_SAMPLES)
// uint16_t channel_reading[BUFFER_SIZE];
//
// // Generic sequence options
// const struct adc_sequence_options options = {
//     .extra_samplings = CONFIG_SEQUENCE_SAMPLES - 1,
//     .interval_us = 0,
// };
//
// // Global sequence structure
// struct adc_sequence sequence = {
//     .buffer = channel_reading,
//     .channels = 0, // Will be set dynamically
//     .buffer_size = sizeof(channel_reading),
//     .resolution = CONFIG_SEQUENCE_RESOLUTION,
//     .oversampling = CONFIG_SEQUENCE_OVERSAMPLING,
//     .options = &options,
// };
//
// // --- Main Logic ---
// int main(void) {
//
//   if (!device_is_ready(adc_dev)) {
//     printf("ADC controller device not ready\n");
//     return 0;
//   }
//
//   // 1. SETUP: Configure all channels once at startup
//   for (size_t i = 0U; i < CHANNEL_COUNT; i++) {
//
//     int err = adc_channel_setup(adc_dev, &channel_cfgs[i]);
//     if (err < 0) {
//       printf("Could not setup channel #%d (%d)\n", channel_cfgs[i].channel_id,
//              err);
//       return 0;
//     }
//   }
//
//   uint32_t count = 0;
//   while (1) {
//     printf("--- Scan Cycle [%u] ---\n", count++);
//
//     /* 2. MANUAL ITERATION: Loop through channels one by one */
//     for (size_t i = 0; i < CHANNEL_COUNT; i++) {
//       const uint8_t hw_channel_id = channel_cfgs[i].channel_id;
//       // Dynamic Configuration: Point sequence to ONLY the current channel's bit
//       // BIT(0) for first pass, BIT(2) for second pass
//       sequence.channels = BIT(hw_channel_id);
//
//       // Perform the read for this single channel
//       int err = adc_read(adc_dev, &sequence);
//       if (err < 0) {
//         printf("Could not read channel %d (%d)\n", hw_channel_id, err);
//         continue;
//       }
//
//       // 3. PROCESS: Handle the result (using the same logic as before)
//       int32_t val_mv = (int32_t)channel_reading[0];
//
//       int err_conv = adc_raw_to_millivolts(VREFS_MV[i], ADC_GAIN_1_2,
//                                            CONFIG_SEQUENCE_RESOLUTION, &val_mv);
//
//       if (err_conv < 0 || VREFS_MV[i] == 0) {
//         printf("- Chan %d: raw %d (mV N/A)\n", hw_channel_id,
//                channel_reading[0]);
//       } else {
//         printf("- Chan %d: raw %d -> %d mV\n", hw_channel_id,
//                channel_reading[0], val_mv);
//       }
//     }
//
//     k_msleep(2000); // Sleep before next full scan cycle
//   }
//   return 0;
// }

// /* ADC node from the devicetree. */
// #define ADC_NODE DT_ALIAS(adc0)
//
// /* Auxiliary macro to obtain channel vref, if available. */
// #define CHANNEL_VREF(node_id) DT_PROP_OR(node_id, zephyr_vref_mv, 0)
//
// /* ADC device specified in devicetree. */
// static const struct device *adc = DEVICE_DT_GET(ADC_NODE);
//
// // #define TEMP_SENSOR_LABEL DT_ALIAS(die_temp0)
//
// /* Common initialization for all channels */
// static struct adc_channel_cfg channel_cfgs[] = {
//     // {
//     //     .channel_id = 0,
//     //     .gain = ADC_GAIN_1_2,
//     //     .reference = ADC_REF_EXTERNAL0,
//     //     .differential = 0,
//     //     .acquisition_time =
//     //         ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131), //
//     ADC_ACQ_TIME_DEFAULT,
//     // },
//        {
//     .channel_id = 2,
//            .gain = ADC_GAIN_1_2,
//            .reference = ADC_REF_EXTERNAL0,
//     .differential = 0,
//            .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131)
//            //ADC_ACQ_TIME_DEFAULT,
//
//        },
//     //   {
//     // .channel_id = 10,
//     //       .gain = ADC_GAIN_1_2,
//     //       .reference = ADC_REF_EXTERNAL0,
//     // .differential = 0,
//     //       .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
//     //       //ADC_ACQ_TIME_DEFAULT,
//     //   },
//     //   {
//     // .channel_id = 11,
//     //       .gain = ADC_GAIN_1_2,
//     //       .reference = ADC_REF_EXTERNAL0,
//     // .differential = 0,
//     //       .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
//     //       //ADC_ACQ_TIME_DEFAULT,
//     //   },
//
// };
//
// /* Data array of ADC channels for the specified ADC. */
// // static const struct adc_channel_cfg channel_cfgs[] = {
// // 	DT_FOREACH_CHILD_SEP(ADC_NODE, ADC_CHANNEL_CFG_DT, (,))};
// //
// // /* Data array of ADC channel voltage references. */
// // static uint32_t vrefs_mv[] = {DT_FOREACH_CHILD_SEP(ADC_NODE, CHANNEL_VREF,
// // (,))};
//
// static uint32_t vrefs_mv[] = {1800, 1800, 1800, 1800};
//
// /* Get the number of channels defined on the DTS. */
// #define CHANNEL_COUNT ARRAY_SIZE(channel_cfgs)
//
// #define BUFFER_SIZE (CONFIG_SEQUENCE_SAMPLES * CHANNEL_COUNT)
//
//   /* Options for the sequence sampling. */
//   // const struct adc_sequence_options options = {
//   //     .extra_samplings = CONFIG_SEQUENCE_SAMPLES - 1,
//   //     .interval_us = 0,
//   // };
//
//   uint16_t channel_reading[BUFFER_SIZE];
//   struct adc_sequence sequence = {
//       .buffer = channel_reading,
//       .channels = 0,
//       /* buffer size in bytes, not number of samples */
//       .buffer_size = sizeof(channel_reading),
//       .resolution = CONFIG_SEQUENCE_RESOLUTION,
//       .oversampling = CONFIG_SEQUENCE_OVERSAMPLING,
// //      .options = &options,
//   };
//
//
// int main(void) {
//
//   int err;
//   uint32_t count = 0;
//
//
//   /* Configure the sampling sequence to be made. */
//   if (!device_is_ready(adc)) {
//
//     printf("ADC controller device %s not ready\n", adc->name);
//     return 0;
//   }
//
//   /* Configure channels individually prior to sampling. */
//   for (size_t i = 0U; i < CHANNEL_COUNT; i++) {
//
//     //sequence.channels |= BIT(channel_cfgs[i].channel_id);
//     err = adc_channel_setup(adc, &channel_cfgs[i]);
//     if (err < 0) {
//
//       printf("Could not setup channel #%d (%d)\n",
//       channel_cfgs[i].channel_id, err); return 0;
//     }
//   }
//   while (1) {
//
//
//     for (int i = 0; i < CHANNEL_COUNT; i++) {
//
//       printf("ADC reading channel[%d]:\n", channel_cfgs[i].channel_id);
//       sequence.channels = BIT(channel_cfgs[i].channel_id);
//       err = adc_read(adc, &sequence);
//       if(err < 0) {
//         printf("Could not read (%d)\n", err);
//         continue;
//       }
//
//       const struct adc_channel_cfg *cfg = &channel_cfgs[i];
//
//       uint8_t res = CONFIG_SEQUENCE_RESOLUTION;
//       int32_t val_mv;
//       /*
//        * Handle differential mode and standard reading from the flat buffer
//        */
//       if (cfg->differential) {
//         val_mv = (int32_t)((int16_t)channel_reading[0]);
//         res -= 1;
//       } else {
//         val_mv = channel_reading[0];
//       }
//
//       printf("raw %" PRId32, val_mv);
//       err = adc_raw_to_millivolts(vrefs_mv[i], cfg->gain, res, &val_mv);
//
//       /* conversion to mV may not be supported, skip if not */
//       if((err < 0) || vrefs_mv[i] == 0) {
//         printf(" (value in mV not available)\n");
//       } else {
//         printf(" = %" PRId32 "mV\n", val_mv);
//       }
//       k_msleep(2000);
//
//     }
//
//
//
//
//
//
//
//
//
//
//     k_msleep(4000);
//
//   //   err = adc_read(adc, &sequence);
//   //   if (err < 0) {
//   //     printf("Could not read (%d)\n", err);
//   //     continue;
//   //   }
//   //
//   //   for (size_t sample_index = 0U; sample_index < CONFIG_SEQUENCE_SAMPLES;
//   //        sample_index++) {
//   //
//   //     for (size_t channel_index = 0U; channel_index < CHANNEL_COUNT;
//   //          channel_index++) {
//   //       int32_t val_mv;
//   //
//   //       // Calculate the flat index into the 1D buffer:
//   //       // index = (sample_index * CHANNEL_COUNT) + channel_index
//   //       size_t flat_index = (sample_index * CHANNEL_COUNT) +
//   channel_index;
//   //       // Get the configuration for the current channel
//   //       const struct adc_channel_cfg *cfg = &channel_cfgs[channel_index];
//   //
//   //       printf("- Sample %" PRId32 ", channel %" PRId32 ": ", sample_index
//   + 1,
//   //              cfg->channel_id);
//   //
//   //       uint8_t res = CONFIG_SEQUENCE_RESOLUTION;
//   //       /*
//   //        * Handle differential mode and standard reading from the flat
//   buffer
//   //        */
//   //       if (cfg->differential) {
//   //         val_mv = (int32_t)((int16_t)channel_reading[flat_index]);
//   //         res -= 1;
//   //       } else {
//   //         val_mv = channel_reading[flat_index];
//   //       }
//   //
//   //       printf("raw %" PRId32, val_mv);
//   //       err = adc_raw_to_millivolts(vrefs_mv[channel_index], cfg->gain,
//   res,
//   //                                   &val_mv);
//   //
//   //       /* conversion to mV may not be supported, skip if not */
//   //       if ((err < 0) || vrefs_mv[channel_index] == 0) {
//   //         printf(" (value in mV not available)\n");
//   //       } else {
//   //         printf(" = %" PRId32 "mV\n", val_mv);
//   //       }
//   //     }
//   //   }
//   }
//   return 0;
// }
//
/* 20000 msec = 1 sec */
// #define SLEEP_TIME_MS   1000
//
// /* The devicetree node identifier for the "led0" alias. */
// #define LED0_NODE DT_ALIAS(led0)
//
// /*
//  * A build error on this line means your board is unsupported.
//  * See the sample documentation for information on how to fix this.
//  */
// static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
//
// int main(void) {
//
// 	int ret;
// 	// bool led_state = true;
// 	//
// 	if(!gpio_is_ready_dt(&led0)) {
//
// 		printf("Could not configure gpio pin!\n");
// 		return 0;
// 	}
// 	//
// 	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
// 	if (ret < 0) {
//
// 		printf("Could not configure gpio pin!\n");
// 		return 0;
// 	}
//
//
// 	while (1) {
//
// 		ret = gpio_pin_toggle_dt(&led0);
// 		if (ret < 0) {
// 			return 0;
// 		}
// 		//led_state = !led_state;
// 		printf("Hello world from GoodEGG board!\n");
// 		k_msleep(SLEEP_TIME_MS);
// 	}
// 	return 0;
// }
