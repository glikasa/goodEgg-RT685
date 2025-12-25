
#include <stdint.h>
#include <stdio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/util.h>

//#define DT_SPEC_AND_COMMA(node_id, prop, idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

#define DT_SPEC_AND_COMMA(node_id,prop,idx) ADC_DT_SPEC_GET_BY_IDX(node_id, idx),
/* Data of ADC io-channels specified in devicetree. */
static struct adc_dt_spec adc_channels[] = {

    DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels, DT_SPEC_AND_COMMA)
};


#define CHANNEL_COUNT ARRAY_SIZE(adc_channels)
#define BUFFER_SIZE (CONFIG_SEQUENCE_SAMPLES * CHANNEL_COUNT) //(CONFIG_SEQUENCE_SAMPLES)
uint16_t channel_reading[BUFFER_SIZE];

// Generic sequence options
const struct adc_sequence_options options = {
    .extra_samplings = CONFIG_SEQUENCE_SAMPLES - 1,
    .interval_us = 0,
};


static uint32_t vrefs_mv[] = {1800, 1800, 1800, 1800};

static struct adc_channel_cfg channel_cfgs[CHANNEL_COUNT] = {
    {
        .channel_id = 2,
        .gain = ADC_GAIN_1_2,
        .reference = ADC_REF_EXTERNAL0,
        .differential = 0,
        .acquisition_time =
            ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131), // ADC_ACQ_TIME_DEFAULT,
        .input_positive = 0,
    },
    {
        .channel_id = 3,
        .gain = ADC_GAIN_1_2,
        .reference = ADC_REF_EXTERNAL0,
        .differential = 0,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
        .input_positive = 2,
        // ADC_ACQ_TIME_DEFAULT,

    },
      {
    .channel_id = 10,
          .gain = ADC_GAIN_1_2,
          .reference = ADC_REF_EXTERNAL0,
    .differential = 0,
          .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
      },
      {
    .channel_id = 11,
          .gain = ADC_GAIN_1_2,
          .reference = ADC_REF_EXTERNAL0,
    .differential = 0,
          .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131),
          //ADC_ACQ_TIME_DEFAULT,
      },

};

int main(void) {

    int err;
    uint32_t count = 0;
    uint32_t channel_mask = 0;
    for(int i = 0; i < CHANNEL_COUNT; i++) {

        if(!device_is_ready(adc_channels[i].dev)) {

            printf("ADC controller device %s not ready\n", adc_channels[i].dev->name);
            return -ENODEV;
        }
        // adc_channels[i].channel_cfg.input_positive = adc_channels[i].channel_id;
        // adc_channels[i].channel_cfg.channel_id = adc_channels[i].channel_id;
        // adc_channels[i].channel_cfg.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_TICKS, 131);
        // adc_channels[i].channel_cfg.differential = 0;

        uint32_t chid = adc_channels[i].channel_id;
        uint32_t encoded_chid = chid < 8 ? chid : 0x20 | (chid & 0x7); 
        channel_cfgs[i].channel_id =  chid;
        channel_cfgs[i].input_positive = encoded_chid; //adc_channels[i].channel_id;
        err = adc_channel_setup(adc_channels[i].dev, &channel_cfgs[i]);
        if(err < 0) {
            printf("Could not setup channel %d \n", adc_channels[i].channel_id);
            return -1;

            //printf("Could not setup channel #%d (%d)\n",
            //channel_cfgs[i].channel_id, err); return 0;
        }
        printf("Successfully setup channel %d \n", chid);
        channel_mask |= BIT(chid);
    }

    // Global sequence structure
    struct adc_sequence sequence = {
        .buffer = channel_reading,
        .channels = channel_mask, // Will be set dynamically
        .buffer_size = sizeof(channel_reading),
        .resolution = CONFIG_SEQUENCE_RESOLUTION,
        .oversampling = CONFIG_SEQUENCE_OVERSAMPLING,
        .options = &options,
    };
    while (1) {

        k_msleep(1000);
        err = adc_read(adc_channels[0].dev, &sequence);
        if(err < 0) {
            printf("Could not read (%d)\n", err);
            continue;
        }

        for(size_t sample_index = 0U; sample_index < CONFIG_SEQUENCE_SAMPLES;
             sample_index++) {

          for(size_t channel_index = 0U; channel_index < CHANNEL_COUNT; channel_index++) {

            int32_t val_mv;

            // Calculate the flat index into the 1D buffer:
            // index = (sample_index * CHANNEL_COUNT) + channel_index
            size_t flat_index = (sample_index * CHANNEL_COUNT) + channel_index;
            // Get the configuration for the current channel
            const struct adc_channel_cfg *cfg = &channel_cfgs[channel_index];

            printf("- Sample %" PRId32 ", channel %" PRId32 ": ", sample_index + 1,
                   cfg->channel_id);

            uint8_t res = CONFIG_SEQUENCE_RESOLUTION;
            /*
             * Handle differential mode and standard reading from the flat
      buffer
             */
            val_mv = channel_reading[flat_index];

            printf("raw %" PRId32, val_mv);
            err = adc_raw_to_millivolts(vrefs_mv[channel_index], cfg->gain,res,
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
